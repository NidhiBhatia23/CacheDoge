/*
 * Created on Mon Nov 19 2018
 * Author: Artur Balanuta & Nidhi Bhatia
 * Copyright (c) 2018 Carnegie Mellon University
 */

#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <stdlib.h>

#include "pin.H"

typedef UINT32 CACHE_STATS; // type of cache hit/miss counters

#include "mypin_cache.H"
#include "mycache_conf.H"



//#define CHECK_INTERVAL (10000) // in Millions of Inst
//#define INVALIDATION_RATIO_THRESHOLD (0.333)         // Precentage of Invalidations cause by a pair


// Other Vars
MyCache cache;
uint8_t thread_id_table[4] = {0, 1, 2, 3};  // Randomized at start
uint64_t exec_time[4];
uint64_t exec_inst_access[4];
uint64_t exec_data_access[4];

uint64_t invalidation_table_l1[4][4];
uint64_t invalidation_table_l1_sum[4][4];
uint64_t invalidation_table_l1_mem[4][4];
uint64_t core_migrations = 0;

KNOB<bool> KnobMigrationEnabled(KNOB_MODE_WRITEONCE, "pintool", "mig",
    "false", "Cache Migration Mechanism");

KNOB<unsigned int> KnobCheckInterval(KNOB_MODE_WRITEONCE, "pintool", "ci",
    "10000", "CHECK_INTERVAL");

KNOB<float> KnobInvalidationRatio(KNOB_MODE_WRITEONCE, "pintool", "irt", 
    "0.333", "INVALIDATION_RATIO_THRESHOLD");

KNOB<float> KnobMemoryRatio(KNOB_MODE_WRITEONCE, "pintool", "mt", 
    "0.0", "The amount of memory to keep between each migration");


#define CACHE_DOGE_ENABLED (KnobMigrationEnabled)
#define CHECK_INTERVAL (KnobCheckInterval) // in Millions of Inst
#define INVALIDATION_RATIO_THRESHOLD (KnobInvalidationRatio)         // Precentage of Invalidations cause by a pair
#define MIGRATION_MEMORY_ALIASING (KnobMemoryRatio)
//        Dst|
//___________| DstCore 0 | DstCore 1 | DstCore 2 | DstCore 3 |
//SrcCore 0 |     X     |           |           |           |
//SrcCore 1 |           |     X     |           |           |
//SrcCore 2 |           |           |     X     |           |
//SrcCore 3 |           |           |           |     X     |

LOCALFUN VOID InsRef(ADDRINT addr, THREADID tid);
LOCALFUN VOID MemRefMulti(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid);
LOCALFUN VOID MemRefSingle(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid);
LOCALFUN VOID Ul2Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, VIRTUALID vid);
LOCALFUN VOID Ul3Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, VIRTUALID vid);
LOCALFUN VOID Instruction(INS ins, VOID *v);
LOCALFUN VOID Fini(int code, VOID * v);
THREADID getVirtualId(THREADID thread_id);
void score_board(uint64_t mat[4][4]);
bool check_conflicts();
string humanize(uint64_t var);


bool check_conflicts()
{

    if (!CACHE_DOGE_ENABLED){
        return false;
    }

    uint64_t triangle[4][4] = {0};

    //        Dst|
    //___________| DstCore 0 | DstCore 1 | DstCore 2 | DstCore 3 |
    //SrcCore 0 |     X     |           |           |           |
    //SrcCore 1 |           |     X     |           |           |
    //SrcCore 2 |           |           |     X     |           |
    //SrcCore 3 |           |           |           |     X     |
    triangle[0][1] = invalidation_table_l1[0][1] + invalidation_table_l1[1][0];
    triangle[0][2] = invalidation_table_l1[0][2] + invalidation_table_l1[2][0];
    triangle[0][3] = invalidation_table_l1[0][3] + invalidation_table_l1[3][0];

    triangle[1][2] = invalidation_table_l1[1][2] + invalidation_table_l1[2][1];
    triangle[1][3] = invalidation_table_l1[1][3] + invalidation_table_l1[3][1];

    triangle[2][3] = invalidation_table_l1[2][3] + invalidation_table_l1[3][2];

    // std::cerr << endl;
    // score_board(triangle);
    // std::cerr << endl;

    const uint64_t sum =    triangle[0][1] + triangle[0][2] + triangle[0][3] + \
                            triangle[1][2] + triangle[1][3] + triangle[2][3];
    //printf("Sum %s\n", humanize(sum).c_str());

    // Important Ones
    // Core <0-2> <0-3> <1-2> <1-3>
   const uint64_t migration_candidates[4] = {   triangle[0][2], triangle[0][3], \
                                                triangle[1][2], triangle[1][3] };

    uint8_t i_max = 0;
    for (int i = 0; i<4; i++){
        if (migration_candidates[i] > migration_candidates[i_max]) {
            i_max = i;
            //printf("Max is Core %d\n", i_max);
        }
    }

    // Check if migration is relevant
    if (double(migration_candidates[i_max])/double(sum) < INVALIDATION_RATIO_THRESHOLD) {
        return false;
    }

    // score_board(invalidation_table_l1);
    // std::cerr << endl;

    //uint8_t random = sum % 2;
        
    switch(i_max) {

        case 0: // <0-2>
            // We should migrate just one of the cores
            // Randomly decide which core to migrate
            //if (random){ // 0 migrates to 3
                cache.getDL1(0)->Flush(); // Clean Caches
                cache.getDL1(3)->Flush();
                std::swap(thread_id_table[0], thread_id_table[3]);
                //printf("Migrating Cores 0 -> 3 \n");
            // }else{ // migrate 2 to 1
            //     cache.getDL1(2)->Flush(); // Clean Caches
            //     cache.getDL1(1)->Flush();
            //     std::swap(thread_id_table[2], thread_id_table[1]);
            //     // printf("Migrating Cores 2 -> 1 \n");
            // }
            break;

        case 1: // <0-3>
            // Randomly decide which core to migrate
            //if (random){ // 0 migrates to 2
                cache.getDL1(0)->Flush(); // Clean Caches
                cache.getDL1(2)->Flush();
                std::swap(thread_id_table[0], thread_id_table[2]);
                //printf("Migrating Cores 0 -> 2 \n");
            // }else{ // migrate 3 to 1
            //     cache.getDL1(3)->Flush(); // Clean Caches
            //     cache.getDL1(1)->Flush();
            //     std::swap(thread_id_table[3], thread_id_table[1]);
            //     //printf("Migrating Cores 3 -> 1 \n");
            // }
            break;
        
        case 2: // <1-2>
            // Randomly decide which core to migrate
            //if (random){ // 1 migrates to 3
                cache.getDL1(1)->Flush(); // Clean Caches
                cache.getDL1(3)->Flush();
                std::swap(thread_id_table[1], thread_id_table[3]);
                //printf("Migrating Cores 1 -> 3 \n");
            // }else{ // migrate 2 to 0
            //     cache.getDL1(2)->Flush(); // Clean Caches
            //     cache.getDL1(0)->Flush();
            //     std::swap(thread_id_table[2], thread_id_table[0]);
            //     //printf("Migrating Cores 2 -> 0 \n");
            // }
            break;
        
        case 3: // <1-3>
            // Randomly decide which core to migrate
            //if (random){ // 1 migrates to 2
                cache.getDL1(1)->Flush(); // Clean Caches
                cache.getDL1(2)->Flush();
                std::swap(thread_id_table[1], thread_id_table[2]);
                // printf("Migrating Cores 1 -> 2 \n");
            // }else{ // migrate 3 to 1
            //     cache.getDL1(3)->Flush(); // Clean Caches
            //     cache.getDL1(0)->Flush();
            //     std::swap(thread_id_table[3], thread_id_table[0]);
            //     //printf("Migrating Cores 3 -> 0 \n");
            // }
            break;
    }

    //printf("Table [ %d %d %d %d ]\n", thread_id_table[0], thread_id_table[1], thread_id_table[2], thread_id_table[3]);

    // Heep history and reset counters
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            if(core_migrations == 0){
                invalidation_table_l1_sum[i][j] = invalidation_table_l1[i][j];
            }else {
                invalidation_table_l1_sum[i][j] += 
                        invalidation_table_l1[i][j] - invalidation_table_l1_mem[i][j];
            }
            invalidation_table_l1[i][j] *= MIGRATION_MEMORY_ALIASING;   // keep some memory ?
            invalidation_table_l1_mem[i][j] = invalidation_table_l1[i][j]; // needed for correct sum values 
        }
    }

    core_migrations++;

    
    // score_board(invalidation_table_l1_sum);
    // std::cerr << endl;
    // std::cerr << endl;
    // std::cerr << endl;

    return true;
} 



THREADID getVirtualId(THREADID thread_id)
{
    if (CACHE_DOGE_ENABLED) {
        // Access Swap Table
        return thread_id_table[thread_id % 4];
    }
    return (thread_id % 4);
}

// received the virtual id of the thread that is trying to write
void tryInvalidateL1(ADDRINT addr, UINT32 size, VIRTUALID vid){

    for (uint i = 0; i < 4; i++ ){
        if (i != vid)
        {
            UINT32 invds = cache.getDL1(i)->Invalidate(addr, size);
            invalidation_table_l1[vid][i] += invds;
        }
    }

}

// received the virtual id of the thread that is trying to write
void tryInvalidateL2(ADDRINT addr, UINT32 size, VIRTUALID vid){
    
    //std::cout << "tryInvalidateL2 " << vid << endl;
    for (uint i = 0; i < 2; i++ ){
        if (i != vid/2)
        {
            cache.getUL2(i*2)->Invalidate(addr, size);
            //std::cout << "InvalidateL2 " << i*2 << endl;
        }
    }

}


// Accessing Instructions Read Only
LOCALFUN VOID InsRef(ADDRINT addr, THREADID tid)
{
    const UINT32 size = 1; // assuming access does not cross cache lines
    const CACHE_BASE::ACCESS_TYPE accessType = CACHE_BASE::ACCESS_TYPE_LOAD;
    THREADID vid = getVirtualId(tid);
    
    // Update Counters
    exec_inst_access[vid]++;
    exec_time[vid] += L1_CACHE_LATENCY; // Adds L1 cache latency
    // TODO Add size dependent latency

    const BOOL il1Hit = cache.getIL1(vid)->AccessSingleLine(addr, accessType);

    // second level unified Cache
    if ( ! il1Hit) Ul2Access(addr, size, accessType, vid);

    // Check conclicts
    if (CACHE_DOGE_ENABLED & 
        (accumulate(exec_inst_access, exec_inst_access+4, 0) % CHECK_INTERVAL == 0)){
        check_conflicts();
    }
}

// Accessing L1 Data Caches Read or Write
LOCALFUN VOID MemRefMulti(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid)
{
    THREADID vid = getVirtualId(tid);
    
    // Update Counters
    exec_data_access[vid]++;
    exec_time[vid] += L1_CACHE_LATENCY; // Adds L1 cache latency
    // TODO Add size dependent latency

    // first level D-cache
    const BOOL dl1Hit = cache.getDL1(vid)->Access(addr, size, accessType);

    if ( !dl1Hit ){
        tryInvalidateL1(addr, size, vid);
        // second level unified Cache
        Ul2Access(addr, size, accessType, vid);
    }
}

// Accessing L1 Data Caches Read or Write
LOCALFUN VOID MemRefSingle(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, THREADID tid)
{
    THREADID vid = getVirtualId(tid);

    // Update Counters
    exec_data_access[vid]++;
    exec_time[vid] += L1_CACHE_LATENCY; // Adds L1 cache latency
    // TODO Add size dependent latency

    // first level D-cache
    const BOOL dl1Hit = cache.getDL1(vid)->AccessSingleLine(addr, accessType);

    if ( !dl1Hit ){
        tryInvalidateL1(addr, size, vid);
        // second level unified Cache
        Ul2Access(addr, size, accessType, vid);
    }
}

// Accessing L2/L3 Data Caches Read or Write
LOCALFUN VOID Ul2Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, VIRTUALID vid)
{
    
    // Update Counters
    exec_time[vid] += L2_CACHE_LATENCY; // Adds L2 cache latency

    // second level cache
    // const BOOL ul2Hit = ul2.Access(addr, size, accessType);
    const BOOL ul2Hit = cache.getUL2(vid)->Access(addr, size, accessType);

    if ( !ul2Hit ) {
        tryInvalidateL2(addr, size, vid);

        // third level unified cache
        Ul3Access(addr, size, accessType, vid);
    }
}

// Accessing L2/L3 Data Caches Read or Write
LOCALFUN VOID Ul3Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType, VIRTUALID vid)
{
    // Update Counters
    exec_time[vid] += L3_CACHE_LATENCY; // Adds L3 cache latency

    // third level cache
    const BOOL ul3Hit = cache.getUL3(vid)->Access(addr, size, accessType);

    if ( !ul3Hit ) {
        // Update Counters
        exec_time[vid] += RAM_LATENCY; // Adds RAM cache latency
    }
}



LOCALFUN VOID Instruction(INS ins, VOID *v)
{
    // all instruction fetches access I-cache
    INS_InsertCall(
        ins,
        IPOINT_BEFORE, 
        (AFUNPTR)InsRef,
        IARG_INST_PTR,
        IARG_THREAD_ID,
        IARG_END);

    if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins))
    {
        const UINT32 size = INS_MemoryReadSize(ins);
        const AFUNPTR countFun = (size <= 4 ? (AFUNPTR) MemRefSingle : (AFUNPTR) MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(
            ins, 
            IPOINT_BEFORE,
            countFun,
            IARG_MEMORYREAD_EA,
            IARG_MEMORYREAD_SIZE,
            IARG_UINT32,
            CACHE_BASE::ACCESS_TYPE_LOAD,
            IARG_THREAD_ID,
            IARG_END);
    }

    if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins))
    {
        const UINT32 size = INS_MemoryWriteSize(ins);
        const AFUNPTR countFun = (size <= 4 ? (AFUNPTR) MemRefSingle : (AFUNPTR) MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE,
            countFun,
            IARG_MEMORYWRITE_EA,
            IARG_MEMORYWRITE_SIZE,
            IARG_UINT32,
            CACHE_BASE::ACCESS_TYPE_STORE,
            IARG_THREAD_ID,
            IARG_END);
    }
}


string humanize(uint64_t var)
{
    static const char sz[] = {' ', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
    int index = 0;
    std::ostringstream out;

    while(var > 1000){
        var /= 1000;
        index++;
    }
    
    out << var << sz[index];

    return out.str();
}

void score_board(uint64_t mat[4][4]){
    //        Dst|
    //___________| DstCore 0 | DstCore 1 | DstCore 2 | DstCore 3 |
    //SrcCore 0 |     X     |           |           |           |
    //SrcCore 1 |           |     X     |           |           |
    //SrcCore 2 |           |           |     X     |           |
    //SrcCore 3 |           |           |           |     X     |
    printf("___________| DstCore 0 | DstCore 1 | DstCore 2 | DstCore 3 |\n");
    for(int i = 0; i < 4; i++ ){
        printf(" SrcCore %d | %9s | %9s | %9s | %9s |\n", i,
            humanize(mat[i][0]).c_str(), \
            humanize(mat[i][1]).c_str(), \
            humanize(mat[i][2]).c_str(), \
            humanize(mat[i][3]).c_str() );
    }
}


LOCALFUN VOID Fini(int code, VOID * v)
{   
    // std::cerr << *cache.getIL1(0);
    // std::cerr << *cache.getIL1(1);
    // std::cerr << *cache.getIL1(2);
    // std::cerr << *cache.getIL1(3);

    // std::cerr << *cache.getDL1(0);
    // std::cerr << *cache.getDL1(1);
    // std::cerr << *cache.getDL1(2);
    // std::cerr << *cache.getDL1(3);
    
    // std::cerr << *cache.getUL2(0);
    // std::cerr << *cache.getUL2(2);

    // std::cerr << *cache.getUL3(0);

    uint64_t total_delay = 0;
    uint64_t total_miss = 0;
    uint64_t exec_time_sum = accumulate(exec_time, exec_time+4, 0);
    uint64_t exec_inst_access_sum = accumulate(exec_inst_access, exec_inst_access+4, 0);
    uint64_t exec_data_access_sum = accumulate(exec_data_access, exec_data_access+4, 0);
    
    for(int i = 0; i < 4; i++ ){

        total_delay += exec_time[i];

        std::cerr << "Core" << i << endl;
        std::cerr << "\tAcces Delay Sum: " << humanize(exec_time[i]) << " Cycles" << endl;
        std::cerr << "\tInst Access Sum: " << humanize(exec_inst_access[i]) << endl;
        std::cerr << "\tData Access Sum: " << humanize(exec_data_access[i]) << endl;
        if (exec_inst_access[i] > 0){
            //load misses per kilo-instructions (MPKI)
            uint64_t miss = cache.getIL1(i)->Misses() + cache.getDL1(i)->Misses();
            std::cerr << "\tL1 MPKI: " << miss * 1.0 / (exec_inst_access[i]/1000) << endl;
            total_miss += miss;
            
            //AVG Delay per Data Access (ADPA)
            std::cerr << "\tDPKA: " << exec_time[i] * 1.0 / exec_data_access[i] << endl;
        }

    }
    std::cerr << endl;

    //std::cerr << "Total Acces Delay Sum: " << humanize(total_delay) << " Cycles" << endl;
    std::cerr << "Total Acces Delay Sum: " << total_delay << " Cycles" << endl;
    std::cerr << "Total MPKI AVG: " << (total_miss * 1.0) / (exec_inst_access_sum/1000) << endl;
    std::cerr << "Total DPKA: " <<  exec_time_sum * 1.0 / exec_data_access_sum << endl;
    std::cerr << "Core Migrations is " << (CACHE_DOGE_ENABLED?"Enabled":"Disabled") << endl;
    std::cerr << "Total Core Migrations: " << core_migrations << endl;
    std::cerr << endl;

    score_board(invalidation_table_l1_sum);
    std::cerr << endl;
}

INT32 Usage()
{
    cerr << "This tool is a multicore cache simulator" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

void randomThreadAlloc(uint8_t tmp[4])
{   
    srand(time(0)); // set seed
    tmp[0] = rand() % 4;
    tmp[1] = rand() % 4;
    tmp[2] = rand() % 4;
    tmp[3] = rand() % 4;
    
    while (  tmp[1] == tmp[0]) tmp[1]  = rand() % 4;

    while ( (tmp[2] == tmp[0]) | (tmp[2] == tmp[1]) ) tmp[2] = rand() % 4;
    
    while ( (tmp[3] == tmp[0]) | (tmp[3] == tmp[1]) | (tmp[3] == tmp[2]) ) tmp[3] = rand() % 4;

}

GLOBALFUN int main(int argc, char *argv[])
{

    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }


    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    if (!CACHE_DOGE_ENABLED)    randomThreadAlloc(thread_id_table); // get random thread allocation

    std::cerr << "Core Migrations is " << (CACHE_DOGE_ENABLED?"Enabled":"Disabled") << endl;
    std::cerr << "Check Interval : " << CHECK_INTERVAL << endl;
    std::cerr << "Invalidation Ratio is : " << INVALIDATION_RATIO_THRESHOLD << endl;
    std::cerr << "Migrations Memory Aliasing is " << MIGRATION_MEMORY_ALIASING << endl;
    printf("Initial Core VID Config: [ %d %d %d %d ]\n", thread_id_table[0], thread_id_table[1], thread_id_table[2], thread_id_table[3]);

    // Never returns
    PIN_StartProgram();

    return 0; // make compiler happy
}


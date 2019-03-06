
#include "../mypin_cache.H"


// CPU Intel Core 2 quad - 6600 @2.4Ghz Characteristics (http://www.cpu-world.com/CPUs/Core_2/Intel-Core%202%20Quad%20Q6600%20HH80562PH0568M%20(BX80562Q6600).html)
// .42 ns/cycle

#define L1_CACHE_LATENCY         5
#define L2_CACHE_LATENCY         12
#define L3_CACHE_LATENCY         267
#define RAM_LATENCY              0
//Modified to 204 cycles to account for 51ns + 42 cycles

//Defines Cache Configurations
namespace ITLR
{
    // instruction Top Level registers: 4 kB pages, 32 entries, fully associative
    const UINT32 lineSize = 1*KILO;
    const UINT32 cacheSize = 32 * lineSize;
    const UINT32 associativity = 32;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

namespace DTLR
{
    // data Top level registers: 4 kB pages, 32 entries, fully associative
    const UINT32 lineSize = 1*KILO;
    const UINT32 cacheSize = 32 * lineSize;
    const UINT32 associativity = 32;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

namespace IL1
{
    // 1st level instruction cache: 32 kB, 32 B lines, 32-way associative
    const UINT32 cacheSize = 32*KILO;
    //const UINT32 lineSize = 32; -- Changed by Nidhi to 64B
    const UINT32 lineSize = 64;
    //const UINT32 associativity = 32; -- Changed by Nidhi to 8
    const UINT32 associativity = 8;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

namespace DL1
{
    // 1st level data cache: 32 kB, 32 B lines, 32-way associative
    const UINT32 cacheSize = 32*KILO;
    //const UINT32 lineSize = 32; -- Changed by Nidhi to 64B
    const UINT32 lineSize = 64;
    //const UINT32 associativity = 32; -- Changed by Nidhi to 8
    const UINT32 associativity = 8;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

namespace UL2
{
    // 2nd level unified cache: 4 MB, 64 B lines, direct mapped
    const UINT32 cacheSize = 4*KILO*KILO;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 16;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);

    typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
}

namespace UL3
{
    // 3rd level unified cache: 1GB, 64 B lines, direct mapped
    const UINT32 cacheSize = GIGA;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 1; 
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);

    typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
}
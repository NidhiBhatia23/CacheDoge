
//#include "arch/mycache_conf_baseline.H"
//#include "arch/mycache_conf_skylake.H"
#include "arch/mycache_conf_knightsLanding.H"

typedef THREADID VIRTUALID;

class MyCache
{
    private:

        // Level 1 Instruction Cache
        IL1::CACHE* il1[4] = {
            new IL1::CACHE("L1I Core0 Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity),
            new IL1::CACHE("L1I Core1 Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity),
            new IL1::CACHE("L1I Core2 Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity),
            new IL1::CACHE("L1I Core3 Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity),
        };
        
        // Level 1 Data Cache
        DL1::CACHE* dl1[4] = {
            new DL1::CACHE("L1D Core0 Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity),
            new DL1::CACHE("L1D Core1 Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity),
            new DL1::CACHE("L1D Core2 Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity),
            new DL1::CACHE("L1D Core3 Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity),
        };
        
        // Level 2 Unified Cache        
        UL2::CACHE* ul2[2] = {
            new UL2::CACHE("L2 Left  Cache", UL2::cacheSize, UL2::lineSize, UL2::associativity),
            new UL2::CACHE("L2 Right Cache", UL2::cacheSize, UL2::lineSize, UL2::associativity),
        };

        // Level 3 Unified Cache
        UL3::CACHE* ul3 = new UL3::CACHE("L3 Unified Cache", UL3::cacheSize, \
            UL3::lineSize, UL3::associativity);
    
    public:

        IL1::CACHE* getIL1(VIRTUALID id){
            return il1[id];
        }

        DL1::CACHE* getDL1(VIRTUALID id){
            return dl1[id];
        }
        
        UL2::CACHE* getUL2(VIRTUALID id){
            return ul2[id/2];
        }
        
        UL3::CACHE* getUL3(VIRTUALID id){
            return ul3;
        }

};


#include "../mypin_cache.H"

// CPU Intel Knights Landing - 1.4GHz
// .71 ns/cycle
// http://mrfunk.info/?page_id=181
// https://www.agner.org/optimize/blog/read.php?i=772&v=t

#define L1_CACHE_LATENCY            2
#define L2_CACHE_LATENCY            10
#define L3_CACHE_LATENCY            212     // Has no L3 Cache
#define RAM_LATENCY 		        0       // Already accounted in L3

//Defines Cache Configurations
namespace IL1
{
    // 1st level instruction cache: 32 kB, 32 B lines, 8-way associative
    const UINT32 cacheSize = 32*KILO;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 8;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

namespace DL1
{
    // 1st level data cache: 32 kB, 32 B lines, 8-way associative
    const UINT32 cacheSize = 32*KILO;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 8;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

// namespace UL2
// {
//     // 2nd level unified cache: 1 MB, 64 B lines, 
//     const UINT32 cacheSize = 1024*KILO;
//     const UINT32 lineSize = 64;
//     const UINT32 associativity = 1;
//     const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

//     const UINT32 max_sets = cacheSize / (lineSize * associativity);

//     typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
// }

namespace UL2
{
    // 1st level data cache: 32 kB, 32 B lines, 8-way associative
    const UINT32 cacheSize = 1*MEGA;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 4;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}

// namespace UL3
// {
//     // 3rd level unified cache: 8 MB, 64 B lines
//     const UINT32 cacheSize = 8*MEGA;
//     const UINT32 lineSize = 64;
//     const UINT32 associativity = 1; 
//     const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

//     const UINT32 max_sets = cacheSize / (lineSize * associativity);

//     typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
// }

namespace UL3
{
    // 1st level data cache: 32 kB, 32 B lines, 8-way associative
    const UINT32 cacheSize = 1*GIGA;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 16;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}
/*
 * Created on Mon Nov 19 2018
 * Author: Artur Balanuta & Nidhi Bhatia
 * Copyright (c) 2018 Carnegie Mellon University
 */

/*! @file
 *  This file contains a configurable cache class
 */

#ifndef MYPIN_CACHE_H
#define MYPIN_CACHE_H

#include <string>
#include <iostream>

#include "pin_util.H"

/*!
 *  @brief Checks if n is a power of 2.
 *  @returns true if n is power of 2
 */
static inline bool IsPower2(UINT32 n)
{
    return ((n & (n - 1)) == 0);
}

/*!
 *  @brief Computes floor(log2(n))
 *  Works by finding position of MSB set.
 *  @returns -1 if n == 0.
 */
static inline INT32 FloorLog2(UINT32 n)
{
    INT32 p = 0;

    if (n == 0) return -1;

    if (n & 0xffff0000) { p += 16; n >>= 16; }
    if (n & 0x0000ff00)	{ p +=  8; n >>=  8; }
    if (n & 0x000000f0) { p +=  4; n >>=  4; }
    if (n & 0x0000000c) { p +=  2; n >>=  2; }
    if (n & 0x00000002) { p +=  1; }

    return p;
}

/*!
 *  @brief Computes floor(log2(n))
 *  Works by finding position of MSB set.
 *  @returns -1 if n == 0.
 */
static inline INT32 CeilLog2(UINT32 n)
{
    return FloorLog2(n - 1) + 1;
}

/*!
 *  @brief Cache tag - self clearing on creation
 */
class CACHE_TAG
{
  private:
    ADDRINT _tag;

  public:
    CACHE_TAG(ADDRINT tag = 0) { _tag = tag; }
    bool operator==(const CACHE_TAG &right) const { return _tag == right._tag; }
    operator ADDRINT() const { return _tag; }
};


/*!
 * Everything related to cache sets
 */
namespace CACHE_SET
{

/*!
 *  @brief Cache set direct mapped
 */
class DIRECT_MAPPED
{
  private:
    CACHE_TAG _tag;

  public:
    DIRECT_MAPPED(UINT32 associativity = 1) { ASSERTX(associativity == 1); }

    VOID SetAssociativity(UINT32 associativity) { ASSERTX(associativity == 1); }
    UINT32 GetAssociativity(UINT32 associativity) { return 1; }

    UINT32 Find(CACHE_TAG tag) { return(_tag == tag); }
    VOID Replace(CACHE_TAG tag) { _tag = tag; }
    VOID Flush() { _tag = 0; }
    
    // Deletes the specified TAG in the set
    bool Delete(CACHE_TAG tag) {
        if(_tag == tag){
            _tag = 0;
            return true;
        }
        return false;
    }
};

/*!
 *  @brief Cache set with round robin replacement
 */
template <UINT32 MAX_ASSOCIATIVITY = 4>
class ROUND_ROBIN
{
  private:
    CACHE_TAG _tags[MAX_ASSOCIATIVITY];
    UINT32 _tagsLastIndex;
    UINT32 _nextReplaceIndex;

  public:
    ROUND_ROBIN(UINT32 associativity = MAX_ASSOCIATIVITY)
      : _tagsLastIndex(associativity - 1)
    {
        ASSERTX(associativity <= MAX_ASSOCIATIVITY);
        _nextReplaceIndex = _tagsLastIndex;

        for (INT32 index = _tagsLastIndex; index >= 0; index--)
        {
            _tags[index] = CACHE_TAG(0);
        }
    }

    VOID SetAssociativity(UINT32 associativity)
    {
        ASSERTX(associativity <= MAX_ASSOCIATIVITY);
        _tagsLastIndex = associativity - 1;
        _nextReplaceIndex = _tagsLastIndex;
    }
    UINT32 GetAssociativity(UINT32 associativity) { return _tagsLastIndex + 1; }

    UINT32 Find(CACHE_TAG tag)
    {
        bool result = true;

        for (INT32 index = _tagsLastIndex; index >= 0; index--)
        {
            // this is an ugly micro-optimization, but it does cause a
            // tighter assembly loop for ARM that way ...
            if(_tags[index] == tag) goto end;
        }
        result = false;

        end: return result;
    }

    VOID Replace(CACHE_TAG tag)
    {
        // g++ -O3 too dumb to do CSE on following lines?!
        const UINT32 index = _nextReplaceIndex;

        _tags[index] = tag;
        // condition typically faster than modulo
        _nextReplaceIndex = (index == 0 ? _tagsLastIndex : index - 1);
    }

    VOID Flush()
    {
      for (INT32 index = _tagsLastIndex; index >= 0; index--)
      {
	    _tags[index] = 0;
      }
      _nextReplaceIndex=_tagsLastIndex;
    }

    // Deletes the specified TAG in the set
    bool Delete(CACHE_TAG tag)
    {
        for (INT32 index = _tagsLastIndex; index >= 0; index--)
        {
            if(_tags[index] == tag)
            {
                _tags[index] = 0;
                return true;
            }
        }
        return false;
    }
};

} // namespace CACHE_SET

namespace CACHE_ALLOC
{
    typedef enum
    {
        STORE_ALLOCATE,
        STORE_NO_ALLOCATE
    } STORE_ALLOCATION;
}

/*!
 *  @brief Generic cache base class; no allocate specialization, no cache set specialization
 */
class CACHE_BASE
{
  public:
    // types, constants
    typedef enum
    {
        ACCESS_TYPE_LOAD,
        ACCESS_TYPE_STORE,
        ACCESS_TYPE_NUM
    } ACCESS_TYPE;

  protected:
    static const UINT32 HIT_MISS_NUM = 2;
    CACHE_STATS _access[ACCESS_TYPE_NUM][HIT_MISS_NUM];
    const std::string _name;

  private:
    // input params
    const UINT32 _cacheSize;
    const UINT32 _lineSize;
    const UINT32 _associativity;
    UINT32 _numberOfFlushes;
    UINT32 _numberOfResets;
    UINT32 _numberOfInvalidations;

    // computed params
    const UINT32 _lineShift;
    const UINT32 _setIndexMask;

    CACHE_STATS SumAccess(bool hit) const
    {
        CACHE_STATS sum = 0;

        for (UINT32 accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
        {
            sum += _access[accessType][hit];
        }

        return sum;
    }

  protected:
    UINT32 NumSets() const { return _setIndexMask + 1; }

  public:
    // constructors/destructors
    CACHE_BASE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity);

    // accessors
    UINT32 CacheSize() const { return _cacheSize; }
    UINT32 LineSize() const { return _lineSize; }
    UINT32 Associativity() const { return _associativity; }
    //
    CACHE_STATS Hits(ACCESS_TYPE accessType) const { return _access[accessType][true];}
    CACHE_STATS Misses(ACCESS_TYPE accessType) const { return _access[accessType][false];}
    CACHE_STATS Accesses(ACCESS_TYPE accessType) const { return Hits(accessType) + Misses(accessType);}
    CACHE_STATS Hits() const { return SumAccess(true);}
    CACHE_STATS Misses() const { return SumAccess(false);}
    CACHE_STATS Accesses() const { return Hits() + Misses();}

    CACHE_STATS Flushes() const { return _numberOfFlushes;}
    CACHE_STATS Invalidations() const { return _numberOfInvalidations;}
    CACHE_STATS Resets() const { return _numberOfResets;}

    VOID SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex) const
    {
        tag = addr >> _lineShift;
        setIndex = tag & _setIndexMask;
    }

    VOID SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex, UINT32 & lineIndex) const
    {
        const UINT32 lineMask = _lineSize - 1;
        lineIndex = addr & lineMask;
        SplitAddress(addr, tag, setIndex);
    }

    VOID IncFlushCounter()
    {
	_numberOfFlushes += 1;
    }

    VOID IncInvalidationsCounter()
    {
	_numberOfInvalidations += 1;
    }

    VOID IncResetCounter()
    {
	_numberOfResets += 1;
    }

    std::ostream & StatsLong(std::ostream & out) const;
};

CACHE_BASE::CACHE_BASE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity)
  : _name(name),
    _cacheSize(cacheSize),
    _lineSize(lineSize),
    _associativity(associativity),
    _lineShift(FloorLog2(lineSize)),
    _setIndexMask((cacheSize / (associativity * lineSize)) - 1)
{

    ASSERTX(IsPower2(_lineSize));
    ASSERTX(IsPower2(_setIndexMask + 1));

    for (UINT32 accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
    {
        _access[accessType][false] = 0;
        _access[accessType][true] = 0;
    }
}

/*!
 *  @brief Stats output method
 */
std::ostream & CACHE_BASE::StatsLong(std::ostream & out) const
{
    const UINT32 headerWidth = 19;
    const UINT32 numberWidth = 10;
    
    out << _name << ":" << std::endl;


    // Access Types, LOAD, STORE
    // for (UINT32 i = 0; i < ACCESS_TYPE_NUM; i++)
    // {
    //     const ACCESS_TYPE accessType = ACCESS_TYPE(i);

    //     //ignore Instruction caches
    //     if (accessType == ACCESS_TYPE_STORE && Accesses(accessType) == 0) continue;

    //     std::string type(accessType == ACCESS_TYPE_LOAD ? "Load " : "Store");

    //     out << StringString(type + " Hits:      ", headerWidth) << StringInt(Hits(accessType), numberWidth) << std::endl;
    //     out << StringString(type + " Misses:    ", headerWidth) << StringInt(Misses(accessType), numberWidth) << std::endl;
    //     out << StringString(type + " Accesses:  ", headerWidth) << StringInt(Accesses(accessType), numberWidth) << std::endl;
    //     out << StringString(type + " Miss Rate: ", headerWidth) << StringFlt(100.0 * Misses(accessType) / Accesses(accessType), 2, numberWidth-1) << "%" << std::endl;
    //     out << std::endl;
    // }

    out << StringString("Total Hits:      ", headerWidth, ' ') << StringInt(Hits(),     numberWidth) << std::endl;
    out << StringString("Total Misses:    ", headerWidth, ' ') << StringInt(Misses(),   numberWidth) << std::endl;
    out << StringString("Total Accesses:  ", headerWidth, ' ') << StringInt(Accesses(), numberWidth) << std::endl;
    // out << StringString("Total Miss Rate: ", headerWidth, ' ') << StringFlt(100.0 * Misses() / Accesses(), 2, numberWidth-1) << "%" << std::endl;
    out << StringString("Invalidations:   ", headerWidth, ' ') << StringInt(Invalidations(), numberWidth) << std::endl;
    //out << StringString("Flushes:         ", headerWidth, ' ') << StringInt(Flushes(), numberWidth) << std::endl;
    //out << StringString("Stat Resets:     ", headerWidth, ' ') << StringInt(Resets(), numberWidth) << std::endl;
    out << std::endl;

    return out;
}

/// ostream operator for CACHE_BASE
std::ostream & operator<< (std::ostream & out, const CACHE_BASE & cacheBase)
{
    return cacheBase.StatsLong(out);
}

/*!
 *  @brief Templated cache class with specific cache set allocation policies
 *
 *  All that remains to be done here is allocate and deallocate the right
 *  type of cache sets.
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
class CACHE : public CACHE_BASE
{
  private:
    SET _sets[MAX_SETS];

  public:
    // constructors/destructors
    CACHE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity)
      : CACHE_BASE(name, cacheSize, lineSize, associativity)
    {
        ASSERTX(NumSets() <= MAX_SETS);

        for (UINT32 i = 0; i < NumSets(); i++)
        {
            _sets[i].SetAssociativity(associativity);
        }
    }

    // modifiers
    /// Cache access from addr to addr+size-1
    bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType);
    /// Cache access at addr that does not span cache lines
    bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType);
    void Flush();

    /// Invalidates the Cache from addr to addr+size-1
    UINT32 Invalidate(ADDRINT addr, UINT32 size);
    /// Invalidates a Cache addr that does not span cache lines
    bool InvalidateSingleLine(ADDRINT addr);
    
    void ResetStats();
};

/*!
 *  @return true if all accessed cache lines hit
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool CACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType)
{
    const ADDRINT highAddr = addr + size;
    bool allHit = true;

    const ADDRINT lineSize = LineSize();
    const ADDRINT notLineMask = ~(lineSize - 1);
    do
    {
        CACHE_TAG tag;
        UINT32 setIndex;

        SplitAddress(addr, tag, setIndex);

        SET & set = _sets[setIndex];

        bool localHit = set.Find(tag);
        allHit &= localHit;

        // on miss, loads always allocate, stores optionally
        if ( (! localHit) && \
            (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
        {
            set.Replace(tag);
        }

        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr);

    //std::cout << _name << " Addr " << StringHex(addr, 0) << " - " << StringHex(highAddr, 0) << " Hit " << allHit << std::endl;

    _access[accessType][allHit]++;

    return allHit;
}

/*!
 *  @return true if accessed cache line hits
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool CACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType)
{
    CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    SET & set = _sets[setIndex];

    bool hit = set.Find(tag);

    // on miss, loads always allocate, stores optionally
    if ( (! hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
    {
        set.Replace(tag);
    }

    //std::cout << _name << " Addr " << StringHex(addr, 0) << " - " << StringHex(addr, 0) << " Hit " << hit << std::endl;

    _access[accessType][hit]++;

    return hit;
}




template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
UINT32 CACHE<SET,MAX_SETS,STORE_ALLOCATION>::Invalidate(ADDRINT addr, UINT32 size)
{
    const ADDRINT highAddr = addr + size;
    UINT32 invalidated = 0;

    const ADDRINT lineSize = LineSize();
    const ADDRINT notLineMask = ~(lineSize - 1);
    do
    {
        CACHE_TAG tag;
        UINT32 setIndex;

        SplitAddress(addr, tag, setIndex);

        SET & set = _sets[setIndex];

        if ( set.Delete(tag) ){
            invalidated++;
        }

        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr);

    if (invalidated) IncInvalidationsCounter();

    return invalidated;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool CACHE<SET,MAX_SETS,STORE_ALLOCATION>::InvalidateSingleLine(ADDRINT addr)
{
    CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    // Get the set where the address coresponds to
    SET & set = _sets[setIndex];

    bool deleted = set.Delete(tag);

    //std::cout << _name << "Invalidating Addr " << StringHex(addr, 0) << " - " << StringHex(addr, 0) << " Evicted " <<  deleted << std::endl;

    if (deleted) IncInvalidationsCounter();

    return deleted;
}




template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void CACHE<SET,MAX_SETS,STORE_ALLOCATION>::Flush()
{
    for (INT32 index = NumSets(); index >= 0; index--) {
      SET & set = _sets[index];
      set.Flush();
    }
    IncFlushCounter();
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void CACHE<SET,MAX_SETS,STORE_ALLOCATION>::ResetStats()
{
    for (UINT32 accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
    {
	_access[accessType][false] = 0;
	_access[accessType][true] = 0;
    }
    IncResetCounter();
}


// define shortcuts
#define CACHE_DIRECT_MAPPED(MAX_SETS, ALLOCATION) \
    CACHE<CACHE_SET::DIRECT_MAPPED, MAX_SETS, ALLOCATION>
#define CACHE_ROUND_ROBIN(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) \
    CACHE<CACHE_SET::ROUND_ROBIN<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>

#endif // MYPIN_CACHE_H

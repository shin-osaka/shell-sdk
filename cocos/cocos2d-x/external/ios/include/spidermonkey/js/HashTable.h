/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_HashTable_h
#define js_HashTable_h

#include "mozilla/Alignment.h"
#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/Casting.h"
#include "mozilla/HashFunctions.h"
#include "mozilla/MemoryReporting.h"
#include "mozilla/Move.h"
#include "mozilla/Opaque.h"
#include "mozilla/PodOperations.h"
#include "mozilla/ReentrancyGuard.h"
#include "mozilla/TemplateLib.h"
#include "mozilla/TypeTraits.h"
#include "mozilla/UniquePtr.h"

#include "js/Utility.h"

namespace js {

class TempAllocPolicy;
template <class> struct DefaultHasher;
template <class, class> class HashMapEntry;
namespace detail {
    template <class T> class HashTableEntry;
    template <class T, class HashPolicy, class AllocPolicy> class HashTable;
} // namespace detail

/*****************************************************************************/

using Generation = mozilla::Opaque<uint64_t>;

template <class Key,
          class Value,
          class HashPolicy = DefaultHasher<Key>,
          class AllocPolicy = TempAllocPolicy>
class HashMap
{
    typedef HashMapEntry<Key, Value> TableEntry;

    struct MapHashPolicy : HashPolicy
    {
        using Base = HashPolicy;
        typedef Key KeyType;
        static const Key& getKey(TableEntry& e) { return e.key(); }
        static void setKey(TableEntry& e, Key& k) { HashPolicy::rekey(e.mutableKey(), k); }
    };

    typedef detail::HashTable<TableEntry, MapHashPolicy, AllocPolicy> Impl;
    Impl impl;

  public:
    typedef typename HashPolicy::Lookup Lookup;
    typedef TableEntry Entry;

    explicit HashMap(AllocPolicy a = AllocPolicy()) : impl(a)  {}
    MOZ_MUST_USE bool init(uint32_t len = 16) { return impl.init(len); }
    bool initialized() const                  { return impl.initialized(); }

    typedef typename Impl::Ptr Ptr;
    Ptr lookup(const Lookup& l) const                 { return impl.lookup(l); }

    Ptr readonlyThreadsafeLookup(const Lookup& l) const { return impl.readonlyThreadsafeLookup(l); }

    void remove(Ptr p)                                { impl.remove(p); }

    typedef typename Impl::AddPtr AddPtr;
    AddPtr lookupForAdd(const Lookup& l) const {
        return impl.lookupForAdd(l);
    }

    template<typename KeyInput, typename ValueInput>
    MOZ_MUST_USE bool add(AddPtr& p, KeyInput&& k, ValueInput&& v) {
        return impl.add(p,
                        mozilla::Forward<KeyInput>(k),
                        mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput>
    MOZ_MUST_USE bool add(AddPtr& p, KeyInput&& k) {
        return impl.add(p, mozilla::Forward<KeyInput>(k), Value());
    }

    template<typename KeyInput, typename ValueInput>
    MOZ_MUST_USE bool relookupOrAdd(AddPtr& p, KeyInput&& k, ValueInput&& v) {
        return impl.relookupOrAdd(p, k,
                                  mozilla::Forward<KeyInput>(k),
                                  mozilla::Forward<ValueInput>(v));
    }

    typedef typename Impl::Range Range;
    Range all() const                                 { return impl.all(); }

    typedef typename Impl::Enum Enum;

    void clear()                                      { impl.clear(); }

    void finish()                                     { impl.finish(); }

    bool empty() const                                { return impl.empty(); }

    uint32_t count() const                            { return impl.count(); }

    size_t capacity() const                           { return impl.capacity(); }

    size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return impl.sizeOfExcludingThis(mallocSizeOf);
    }
    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this) + impl.sizeOfExcludingThis(mallocSizeOf);
    }

    Generation generation() const {
        return impl.generation();
    }

    /************************************************** Shorthand operations */

    bool has(const Lookup& l) const {
        return impl.lookup(l).found();
    }

    template<typename KeyInput, typename ValueInput>
    MOZ_MUST_USE bool put(KeyInput&& k, ValueInput&& v) {
        AddPtr p = lookupForAdd(k);
        if (p) {
            p->value() = mozilla::Forward<ValueInput>(v);
            return true;
        }
        return add(p, mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput, typename ValueInput>
    MOZ_MUST_USE bool putNew(KeyInput&& k, ValueInput&& v) {
        return impl.putNew(k, mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput, typename ValueInput>
    void putNewInfallible(KeyInput&& k, ValueInput&& v) {
        impl.putNewInfallible(k, mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }

    Ptr lookupWithDefault(const Key& k, const Value& defaultValue) {
        AddPtr p = lookupForAdd(k);
        if (p)
            return p;
        bool ok = add(p, k, defaultValue);
        MOZ_ASSERT_IF(!ok, !p); // p is left false-y on oom.
        (void)ok;
        return p;
    }

    void remove(const Lookup& l) {
        if (Ptr p = lookup(l))
            remove(p);
    }

    void rekeyIfMoved(const Key& old_key, const Key& new_key) {
        if (old_key != new_key)
            rekeyAs(old_key, new_key, new_key);
    }

    bool rekeyAs(const Lookup& old_lookup, const Lookup& new_lookup, const Key& new_key) {
        if (Ptr p = lookup(old_lookup)) {
            impl.rekeyAndMaybeRehash(p, new_lookup, new_key);
            return true;
        }
        return false;
    }

    HashMap(HashMap&& rhs) : impl(mozilla::Move(rhs.impl)) {}
    void operator=(HashMap&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        impl = mozilla::Move(rhs.impl);
    }

  private:
    HashMap(const HashMap& hm) = delete;
    HashMap& operator=(const HashMap& hm) = delete;

    friend class Impl::Enum;
};

/*****************************************************************************/

template <class T,
          class HashPolicy = DefaultHasher<T>,
          class AllocPolicy = TempAllocPolicy>
class HashSet
{
    struct SetOps : HashPolicy
    {
        using Base = HashPolicy;
        typedef T KeyType;
        static const KeyType& getKey(const T& t) { return t; }
        static void setKey(T& t, KeyType& k) { HashPolicy::rekey(t, k); }
    };

    typedef detail::HashTable<const T, SetOps, AllocPolicy> Impl;
    Impl impl;

  public:
    typedef typename HashPolicy::Lookup Lookup;
    typedef T Entry;

    explicit HashSet(AllocPolicy a = AllocPolicy()) : impl(a)  {}
    MOZ_MUST_USE bool init(uint32_t len = 16) { return impl.init(len); }
    bool initialized() const                  { return impl.initialized(); }

    typedef typename Impl::Ptr Ptr;
    Ptr lookup(const Lookup& l) const                 { return impl.lookup(l); }

    Ptr readonlyThreadsafeLookup(const Lookup& l) const { return impl.readonlyThreadsafeLookup(l); }

    void remove(Ptr p)                                { impl.remove(p); }

    typedef typename Impl::AddPtr AddPtr;
    AddPtr lookupForAdd(const Lookup& l) const        { return impl.lookupForAdd(l); }

    template <typename U>
    MOZ_MUST_USE bool add(AddPtr& p, U&& u) {
        return impl.add(p, mozilla::Forward<U>(u));
    }

    template <typename U>
    MOZ_MUST_USE bool relookupOrAdd(AddPtr& p, const Lookup& l, U&& u) {
        return impl.relookupOrAdd(p, l, mozilla::Forward<U>(u));
    }

    typedef typename Impl::Range Range;
    Range all() const                                 { return impl.all(); }

    typedef typename Impl::Enum Enum;

    void clear()                                      { impl.clear(); }

    void finish()                                     { impl.finish(); }

    bool empty() const                                { return impl.empty(); }

    uint32_t count() const                            { return impl.count(); }

    size_t capacity() const                           { return impl.capacity(); }

    size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return impl.sizeOfExcludingThis(mallocSizeOf);
    }
    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this) + impl.sizeOfExcludingThis(mallocSizeOf);
    }

    Generation generation() const {
        return impl.generation();
    }

    /************************************************** Shorthand operations */

    bool has(const Lookup& l) const {
        return impl.lookup(l).found();
    }

    template <typename U>
    MOZ_MUST_USE bool put(U&& u) {
        AddPtr p = lookupForAdd(u);
        return p ? true : add(p, mozilla::Forward<U>(u));
    }

    template <typename U>
    MOZ_MUST_USE bool putNew(U&& u) {
        return impl.putNew(u, mozilla::Forward<U>(u));
    }

    template <typename U>
    MOZ_MUST_USE bool putNew(const Lookup& l, U&& u) {
        return impl.putNew(l, mozilla::Forward<U>(u));
    }

    template <typename U>
    void putNewInfallible(const Lookup& l, U&& u) {
        impl.putNewInfallible(l, mozilla::Forward<U>(u));
    }

    void remove(const Lookup& l) {
        if (Ptr p = lookup(l))
            remove(p);
    }

    void rekeyIfMoved(const Lookup& old_value, const T& new_value) {
        if (old_value != new_value)
            rekeyAs(old_value, new_value, new_value);
    }

    bool rekeyAs(const Lookup& old_lookup, const Lookup& new_lookup, const T& new_value) {
        if (Ptr p = lookup(old_lookup)) {
            impl.rekeyAndMaybeRehash(p, new_lookup, new_value);
            return true;
        }
        return false;
    }

    void replaceKey(Ptr p, const T& new_value) {
        MOZ_ASSERT(p.found());
        MOZ_ASSERT(*p != new_value);
        MOZ_ASSERT(HashPolicy::hash(*p) == HashPolicy::hash(new_value));
        MOZ_ASSERT(HashPolicy::match(*p, new_value));
        const_cast<T&>(*p) = new_value;
    }

    HashSet(HashSet&& rhs) : impl(mozilla::Move(rhs.impl)) {}
    void operator=(HashSet&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        impl = mozilla::Move(rhs.impl);
    }

  private:
    HashSet(const HashSet& hs) = delete;
    HashSet& operator=(const HashSet& hs) = delete;

    friend class Impl::Enum;
};

/*****************************************************************************/


template <typename Key, size_t zeroBits>
struct PointerHasher
{
    typedef Key Lookup;
    static HashNumber hash(const Lookup& l) {
        size_t word = reinterpret_cast<size_t>(l) >> zeroBits;
        static_assert(sizeof(HashNumber) == 4,
                      "subsequent code assumes a four-byte hash");
#if JS_BITS_PER_WORD == 32
        return HashNumber(word);
#else
        static_assert(sizeof(word) == 8,
                      "unexpected word size, new hashing strategy required to "
                      "properly incorporate all bits");
        return HashNumber((word >> 32) ^ word);
#endif
    }
    static bool match(const Key& k, const Lookup& l) {
        return k == l;
    }
    static void rekey(Key& k, const Key& newKey) {
        k = newKey;
    }
};

template <class Key>
struct DefaultHasher
{
    typedef Key Lookup;
    static HashNumber hash(const Lookup& l) {
        return l;
    }
    static bool match(const Key& k, const Lookup& l) {
        return k == l;
    }
    static void rekey(Key& k, const Key& newKey) {
        k = newKey;
    }
};

template <class T>
struct DefaultHasher<T*> : PointerHasher<T*, mozilla::tl::FloorLog2<sizeof(void*)>::value>
{};

template <class T, class D>
struct DefaultHasher<mozilla::UniquePtr<T, D>>
{
    using Lookup = mozilla::UniquePtr<T, D>;
    using PtrHasher = PointerHasher<T*, mozilla::tl::FloorLog2<sizeof(void*)>::value>;

    static HashNumber hash(const Lookup& l) {
        return PtrHasher::hash(l.get());
    }
    static bool match(const mozilla::UniquePtr<T, D>& k, const Lookup& l) {
        return PtrHasher::match(k.get(), l.get());
    }
    static void rekey(mozilla::UniquePtr<T, D>& k, mozilla::UniquePtr<T, D>&& newKey) {
        k = mozilla::Move(newKey);
    }
};

template <>
struct DefaultHasher<double>
{
    typedef double Lookup;
    static HashNumber hash(double d) {
        static_assert(sizeof(HashNumber) == 4,
                      "subsequent code assumes a four-byte hash");
        uint64_t u = mozilla::BitwiseCast<uint64_t>(d);
        return HashNumber(u ^ (u >> 32));
    }
    static bool match(double lhs, double rhs) {
        return mozilla::BitwiseCast<uint64_t>(lhs) == mozilla::BitwiseCast<uint64_t>(rhs);
    }
};

template <>
struct DefaultHasher<float>
{
    typedef float Lookup;
    static HashNumber hash(float f) {
        static_assert(sizeof(HashNumber) == 4,
                      "subsequent code assumes a four-byte hash");
        return HashNumber(mozilla::BitwiseCast<uint32_t>(f));
    }
    static bool match(float lhs, float rhs) {
        return mozilla::BitwiseCast<uint32_t>(lhs) == mozilla::BitwiseCast<uint32_t>(rhs);
    }
};

struct CStringHasher
{
    typedef const char* Lookup;
    static js::HashNumber hash(Lookup l) {
        return mozilla::HashString(l);
    }
    static bool match(const char* key, Lookup lookup) {
        return strcmp(key, lookup) == 0;
    }
};

template <typename HashPolicy>
struct FallibleHashMethods
{
    template <typename Lookup> static bool hasHash(Lookup&& l) { return true; }

    template <typename Lookup> static bool ensureHash(Lookup&& l) { return true; }
};

template <typename HashPolicy, typename Lookup>
static bool
HasHash(Lookup&& l) {
    return FallibleHashMethods<typename HashPolicy::Base>::hasHash(mozilla::Forward<Lookup>(l));
}

template <typename HashPolicy, typename Lookup>
static bool
EnsureHash(Lookup&& l) {
    return FallibleHashMethods<typename HashPolicy::Base>::ensureHash(mozilla::Forward<Lookup>(l));
}

/*****************************************************************************/


template <class Key, class Value>
class HashMapEntry
{
    Key key_;
    Value value_;

    template <class, class, class> friend class detail::HashTable;
    template <class> friend class detail::HashTableEntry;
    template <class, class, class, class> friend class HashMap;

  public:
    template<typename KeyInput, typename ValueInput>
    HashMapEntry(KeyInput&& k, ValueInput&& v)
      : key_(mozilla::Forward<KeyInput>(k)),
        value_(mozilla::Forward<ValueInput>(v))
    {}

    HashMapEntry(HashMapEntry&& rhs)
      : key_(mozilla::Move(rhs.key_)),
        value_(mozilla::Move(rhs.value_))
    {}

    void operator=(HashMapEntry&& rhs) {
        key_ = mozilla::Move(rhs.key_);
        value_ = mozilla::Move(rhs.value_);
    }

    typedef Key KeyType;
    typedef Value ValueType;

    const Key& key() const { return key_; }
    Key& mutableKey() { return key_; }
    const Value& value() const { return value_; }
    Value& value() { return value_; }

  private:
    HashMapEntry(const HashMapEntry&) = delete;
    void operator=(const HashMapEntry&) = delete;
};

} // namespace js

namespace mozilla {

template <typename T>
struct IsPod<js::detail::HashTableEntry<T> > : IsPod<T> {};

template <typename K, typename V>
struct IsPod<js::HashMapEntry<K, V> >
  : IntegralConstant<bool, IsPod<K>::value && IsPod<V>::value>
{};

} // namespace mozilla

namespace js {

namespace detail {

template <class T, class HashPolicy, class AllocPolicy>
class HashTable;

template <class T>
class HashTableEntry
{
    template <class, class, class> friend class HashTable;
    typedef typename mozilla::RemoveConst<T>::Type NonConstT;

    HashNumber keyHash;
    mozilla::AlignedStorage2<NonConstT> mem;

    static const HashNumber sFreeKey = 0;
    static const HashNumber sRemovedKey = 1;
    static const HashNumber sCollisionBit = 1;

    static bool isLiveHash(HashNumber hash)
    {
        return hash > sRemovedKey;
    }

    HashTableEntry(const HashTableEntry&) = delete;
    void operator=(const HashTableEntry&) = delete;
    ~HashTableEntry() = delete;

  public:

    void destroyIfLive() {
        if (isLive())
            mem.addr()->~T();
    }

    void destroy() {
        MOZ_ASSERT(isLive());
        mem.addr()->~T();
    }

    void swap(HashTableEntry* other) {
        if (this == other)
            return;
        MOZ_ASSERT(isLive());
        if (other->isLive()) {
            mozilla::Swap(*mem.addr(), *other->mem.addr());
        } else {
            *other->mem.addr() = mozilla::Move(*mem.addr());
            destroy();
        }
        mozilla::Swap(keyHash, other->keyHash);
    }

    T& get() { MOZ_ASSERT(isLive()); return *mem.addr(); }
    NonConstT& getMutable() { MOZ_ASSERT(isLive()); return *mem.addr(); }

    bool isFree() const    { return keyHash == sFreeKey; }
    void clearLive()       { MOZ_ASSERT(isLive()); keyHash = sFreeKey; mem.addr()->~T(); }
    void clear()           { if (isLive()) mem.addr()->~T(); keyHash = sFreeKey; }
    bool isRemoved() const { return keyHash == sRemovedKey; }
    void removeLive()      { MOZ_ASSERT(isLive()); keyHash = sRemovedKey; mem.addr()->~T(); }
    bool isLive() const    { return isLiveHash(keyHash); }
    void setCollision()               { MOZ_ASSERT(isLive()); keyHash |= sCollisionBit; }
    void unsetCollision()             { keyHash &= ~sCollisionBit; }
    bool hasCollision() const         { return keyHash & sCollisionBit; }
    bool matchHash(HashNumber hn)     { return (keyHash & ~sCollisionBit) == hn; }
    HashNumber getKeyHash() const     { return keyHash & ~sCollisionBit; }

    template <typename... Args>
    void setLive(HashNumber hn, Args&&... args)
    {
        MOZ_ASSERT(!isLive());
        keyHash = hn;
        new(mem.addr()) T(mozilla::Forward<Args>(args)...);
        MOZ_ASSERT(isLive());
    }
};

template <class T, class HashPolicy, class AllocPolicy>
class HashTable : private AllocPolicy
{
    friend class mozilla::ReentrancyGuard;

    typedef typename mozilla::RemoveConst<T>::Type NonConstT;
    typedef typename HashPolicy::KeyType Key;
    typedef typename HashPolicy::Lookup Lookup;

  public:
    typedef HashTableEntry<T> Entry;

    class Ptr
    {
        friend class HashTable;

        Entry* entry_;
#ifdef JS_DEBUG
        const HashTable* table_;
        Generation generation;
#endif

      protected:
        Ptr(Entry& entry, const HashTable& tableArg)
          : entry_(&entry)
#ifdef JS_DEBUG
          , table_(&tableArg)
          , generation(tableArg.generation())
#endif
        {}

      public:
        Ptr()
          : entry_(nullptr)
#ifdef JS_DEBUG
          , table_(nullptr)
          , generation(0)
#endif
        {}

        bool isValid() const {
            return !entry_;
        }

        bool found() const {
            if (isValid())
                return false;
#ifdef JS_DEBUG
            MOZ_ASSERT(generation == table_->generation());
#endif
            return entry_->isLive();
        }

        explicit operator bool() const {
            return found();
        }

        bool operator==(const Ptr& rhs) const {
            MOZ_ASSERT(found() && rhs.found());
            return entry_ == rhs.entry_;
        }

        bool operator!=(const Ptr& rhs) const {
#ifdef JS_DEBUG
            MOZ_ASSERT(generation == table_->generation());
#endif
            return !(*this == rhs);
        }

        T& operator*() const {
#ifdef JS_DEBUG
            MOZ_ASSERT(found());
            MOZ_ASSERT(generation == table_->generation());
#endif
            return entry_->get();
        }

        T* operator->() const {
#ifdef JS_DEBUG
            MOZ_ASSERT(found());
            MOZ_ASSERT(generation == table_->generation());
#endif
            return &entry_->get();
        }
    };

    class AddPtr : public Ptr
    {
        friend class HashTable;
        HashNumber keyHash;
#ifdef JS_DEBUG
        uint64_t mutationCount;
#endif

        AddPtr(Entry& entry, const HashTable& tableArg, HashNumber hn)
          : Ptr(entry, tableArg)
          , keyHash(hn)
#ifdef JS_DEBUG
          , mutationCount(tableArg.mutationCount)
#endif
        {}

      public:
        AddPtr() : keyHash(0) {}
    };

    class Range
    {
      protected:
        friend class HashTable;

        Range(const HashTable& tableArg, Entry* c, Entry* e)
          : cur(c)
          , end(e)
#ifdef JS_DEBUG
          , table_(&tableArg)
          , mutationCount(tableArg.mutationCount)
          , generation(tableArg.generation())
          , validEntry(true)
#endif
        {
            while (cur < end && !cur->isLive())
                ++cur;
        }

        Entry* cur;
        Entry* end;
#ifdef JS_DEBUG
        const HashTable* table_;
        uint64_t mutationCount;
        Generation generation;
        bool validEntry;
#endif

      public:
        Range()
          : cur(nullptr)
          , end(nullptr)
#ifdef JS_DEBUG
          , table_(nullptr)
          , mutationCount(0)
          , generation(0)
          , validEntry(false)
#endif
        {}

        bool empty() const {
#ifdef JS_DEBUG
            MOZ_ASSERT(generation == table_->generation());
            MOZ_ASSERT(mutationCount == table_->mutationCount);
#endif
            return cur == end;
        }

        T& front() const {
            MOZ_ASSERT(!empty());
#ifdef JS_DEBUG
            MOZ_ASSERT(validEntry);
            MOZ_ASSERT(generation == table_->generation());
            MOZ_ASSERT(mutationCount == table_->mutationCount);
#endif
            return cur->get();
        }

        void popFront() {
            MOZ_ASSERT(!empty());
#ifdef JS_DEBUG
            MOZ_ASSERT(generation == table_->generation());
            MOZ_ASSERT(mutationCount == table_->mutationCount);
#endif
            while (++cur < end && !cur->isLive())
                continue;
#ifdef JS_DEBUG
            validEntry = true;
#endif
        }
    };

    class Enum : public Range
    {
        friend class HashTable;

        HashTable& table_;
        bool rekeyed;
        bool removed;

        /* Not copyable. */
        Enum(const Enum&) = delete;
        void operator=(const Enum&) = delete;

      public:
        template<class Map> explicit
        Enum(Map& map) : Range(map.all()), table_(map.impl), rekeyed(false), removed(false) {}

        void removeFront() {
            table_.remove(*this->cur);
            removed = true;
#ifdef JS_DEBUG
            this->validEntry = false;
            this->mutationCount = table_.mutationCount;
#endif
        }

        NonConstT& mutableFront() {
            MOZ_ASSERT(!this->empty());
#ifdef JS_DEBUG
            MOZ_ASSERT(this->validEntry);
            MOZ_ASSERT(this->generation == this->Range::table_->generation());
            MOZ_ASSERT(this->mutationCount == this->Range::table_->mutationCount);
#endif
            return this->cur->getMutable();
        }

        void rekeyFront(const Lookup& l, const Key& k) {
            MOZ_ASSERT(&k != &HashPolicy::getKey(this->cur->get()));
            Ptr p(*this->cur, table_);
            table_.rekeyWithoutRehash(p, l, k);
            rekeyed = true;
#ifdef JS_DEBUG
            this->validEntry = false;
            this->mutationCount = table_.mutationCount;
#endif
        }

        void rekeyFront(const Key& k) {
            rekeyFront(k, k);
        }

        ~Enum() {
            if (rekeyed) {
                table_.gen++;
                table_.checkOverRemoved();
            }

            if (removed)
                table_.compactIfUnderloaded();
        }
    };

    HashTable(HashTable&& rhs)
      : AllocPolicy(rhs)
    {
        mozilla::PodAssign(this, &rhs);
        rhs.table = nullptr;
    }
    void operator=(HashTable&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        if (table)
            destroyTable(*this, table, capacity());
        mozilla::PodAssign(this, &rhs);
        rhs.table = nullptr;
    }

  private:
    HashTable(const HashTable&) = delete;
    void operator=(const HashTable&) = delete;

  private:
    static const size_t CAP_BITS = 30;

  public:
    uint64_t    gen:56;                 // entry storage generation number
    uint64_t    hashShift:8;            // multiplicative hash shift
    Entry*      table;                  // entry storage
    uint32_t    entryCount;             // number of entries in table
    uint32_t    removedCount;           // removed entry sentinels in table

#ifdef JS_DEBUG
    uint64_t     mutationCount;
    mutable bool mEntered;
    mutable struct Stats
    {
        uint32_t        searches;       // total number of table searches
        uint32_t        steps;          // hash chain links traversed
        uint32_t        hits;           // searches that found key
        uint32_t        misses;         // searches that didn't find key
        uint32_t        addOverRemoved; // adds that recycled a removed entry
        uint32_t        removes;        // calls to remove
        uint32_t        removeFrees;    // calls to remove that freed the entry
        uint32_t        grows;          // table expansions
        uint32_t        shrinks;        // table contractions
        uint32_t        compresses;     // table compressions
        uint32_t        rehashes;       // tombstone decontaminations
    } stats;
#   define METER(x) x
#else
#   define METER(x)
#endif

    static const unsigned sMinCapacityLog2 = 2;
    static const unsigned sMinCapacity  = 1 << sMinCapacityLog2;
    static const unsigned sMaxInit      = JS_BIT(CAP_BITS - 1);
    static const unsigned sMaxCapacity  = JS_BIT(CAP_BITS);
    static const unsigned sHashBits     = mozilla::tl::BitSize<HashNumber>::value;

    static const uint8_t sAlphaDenominator = 4;
    static const uint8_t sMinAlphaNumerator = 1; // min alpha: 1/4
    static const uint8_t sMaxAlphaNumerator = 3; // max alpha: 3/4

    static const HashNumber sFreeKey = Entry::sFreeKey;
    static const HashNumber sRemovedKey = Entry::sRemovedKey;
    static const HashNumber sCollisionBit = Entry::sCollisionBit;

    void setTableSizeLog2(unsigned sizeLog2)
    {
        hashShift = sHashBits - sizeLog2;
    }

    static bool isLiveHash(HashNumber hash)
    {
        return Entry::isLiveHash(hash);
    }

    static HashNumber prepareHash(const Lookup& l)
    {
        HashNumber keyHash = ScrambleHashCode(HashPolicy::hash(l));

        if (!isLiveHash(keyHash))
            keyHash -= (sRemovedKey + 1);
        return keyHash & ~sCollisionBit;
    }

    enum FailureBehavior { DontReportFailure = false, ReportFailure = true };

    static Entry* createTable(AllocPolicy& alloc, uint32_t capacity,
                              FailureBehavior reportFailure = ReportFailure)
    {
        static_assert(sFreeKey == 0,
                      "newly-calloc'd tables have to be considered empty");
        if (reportFailure)
            return alloc.template pod_calloc<Entry>(capacity);

        return alloc.template maybe_pod_calloc<Entry>(capacity);
    }

    static Entry* maybeCreateTable(AllocPolicy& alloc, uint32_t capacity)
    {
        static_assert(sFreeKey == 0,
                      "newly-calloc'd tables have to be considered empty");
        return alloc.template maybe_pod_calloc<Entry>(capacity);
    }

    static void destroyTable(AllocPolicy& alloc, Entry* oldTable, uint32_t capacity)
    {
        Entry* end = oldTable + capacity;
        for (Entry* e = oldTable; e < end; ++e)
            e->destroyIfLive();
        alloc.free_(oldTable);
    }

  public:
    explicit HashTable(AllocPolicy ap)
      : AllocPolicy(ap)
      , gen(0)
      , hashShift(sHashBits)
      , table(nullptr)
      , entryCount(0)
      , removedCount(0)
#ifdef JS_DEBUG
      , mutationCount(0)
      , mEntered(false)
#endif
    {}

    MOZ_MUST_USE bool init(uint32_t length)
    {
        MOZ_ASSERT(!initialized());

        if (MOZ_UNLIKELY(length > sMaxInit)) {
            this->reportAllocOverflow();
            return false;
        }

        static_assert((sMaxInit * sAlphaDenominator) / sAlphaDenominator == sMaxInit,
                      "multiplication in numerator below could overflow");
        static_assert(sMaxInit * sAlphaDenominator <= UINT32_MAX - sMaxAlphaNumerator,
                      "numerator calculation below could potentially overflow");

        uint32_t newCapacity =
            (length * sAlphaDenominator + sMaxAlphaNumerator - 1) / sMaxAlphaNumerator;
        if (newCapacity < sMinCapacity)
            newCapacity = sMinCapacity;

        uint32_t roundUp = sMinCapacity, roundUpLog2 = sMinCapacityLog2;
        while (roundUp < newCapacity) {
            roundUp <<= 1;
            ++roundUpLog2;
        }

        newCapacity = roundUp;
        MOZ_ASSERT(newCapacity >= length);
        MOZ_ASSERT(newCapacity <= sMaxCapacity);

        table = createTable(*this, newCapacity);
        if (!table)
            return false;

        setTableSizeLog2(roundUpLog2);
        METER(memset(&stats, 0, sizeof(stats)));
        return true;
    }

    bool initialized() const
    {
        return !!table;
    }

    ~HashTable()
    {
        if (table)
            destroyTable(*this, table, capacity());
    }

  private:
    HashNumber hash1(HashNumber hash0) const
    {
        return hash0 >> hashShift;
    }

    struct DoubleHash
    {
        HashNumber h2;
        HashNumber sizeMask;
    };

    DoubleHash hash2(HashNumber curKeyHash) const
    {
        unsigned sizeLog2 = sHashBits - hashShift;
        DoubleHash dh = {
            ((curKeyHash << sizeLog2) >> hashShift) | 1,
            (HashNumber(1) << sizeLog2) - 1
        };
        return dh;
    }

    static HashNumber applyDoubleHash(HashNumber h1, const DoubleHash& dh)
    {
        return (h1 - dh.h2) & dh.sizeMask;
    }

    bool overloaded()
    {
        static_assert(sMaxCapacity <= UINT32_MAX / sMaxAlphaNumerator,
                      "multiplication below could overflow");
        return entryCount + removedCount >=
               capacity() * sMaxAlphaNumerator / sAlphaDenominator;
    }

    static bool wouldBeUnderloaded(uint32_t capacity, uint32_t entryCount)
    {
        static_assert(sMaxCapacity <= UINT32_MAX / sMinAlphaNumerator,
                      "multiplication below could overflow");
        return capacity > sMinCapacity &&
               entryCount <= capacity * sMinAlphaNumerator / sAlphaDenominator;
    }

    bool underloaded()
    {
        return wouldBeUnderloaded(capacity(), entryCount);
    }

    static bool match(Entry& e, const Lookup& l)
    {
        return HashPolicy::match(HashPolicy::getKey(e.get()), l);
    }

    Entry& lookup(const Lookup& l, HashNumber keyHash, unsigned collisionBit) const
    {
        MOZ_ASSERT(isLiveHash(keyHash));
        MOZ_ASSERT(!(keyHash & sCollisionBit));
        MOZ_ASSERT(collisionBit == 0 || collisionBit == sCollisionBit);
        MOZ_ASSERT(table);
        METER(stats.searches++);

        HashNumber h1 = hash1(keyHash);
        Entry* entry = &table[h1];

        if (entry->isFree()) {
            METER(stats.misses++);
            return *entry;
        }

        if (entry->matchHash(keyHash) && match(*entry, l)) {
            METER(stats.hits++);
            return *entry;
        }

        DoubleHash dh = hash2(keyHash);

        Entry* firstRemoved = nullptr;

        while (true) {
            if (MOZ_UNLIKELY(entry->isRemoved())) {
                if (!firstRemoved)
                    firstRemoved = entry;
            } else {
                if (collisionBit == sCollisionBit)
                    entry->setCollision();
            }

            METER(stats.steps++);
            h1 = applyDoubleHash(h1, dh);

            entry = &table[h1];
            if (entry->isFree()) {
                METER(stats.misses++);
                return firstRemoved ? *firstRemoved : *entry;
            }

            if (entry->matchHash(keyHash) && match(*entry, l)) {
                METER(stats.hits++);
                return *entry;
            }
        }
    }

    Entry& findFreeEntry(HashNumber keyHash)
    {
        MOZ_ASSERT(!(keyHash & sCollisionBit));
        MOZ_ASSERT(table);
        METER(stats.searches++);


        HashNumber h1 = hash1(keyHash);
        Entry* entry = &table[h1];

        if (!entry->isLive()) {
            METER(stats.misses++);
            return *entry;
        }

        DoubleHash dh = hash2(keyHash);

        while (true) {
            MOZ_ASSERT(!entry->isRemoved());
            entry->setCollision();

            METER(stats.steps++);
            h1 = applyDoubleHash(h1, dh);

            entry = &table[h1];
            if (!entry->isLive()) {
                METER(stats.misses++);
                return *entry;
            }
        }
    }

    enum RebuildStatus { NotOverloaded, Rehashed, RehashFailed };

    RebuildStatus changeTableSize(int deltaLog2, FailureBehavior reportFailure = ReportFailure)
    {
        Entry* oldTable = table;
        uint32_t oldCap = capacity();
        uint32_t newLog2 = sHashBits - hashShift + deltaLog2;
        uint32_t newCapacity = JS_BIT(newLog2);
        if (MOZ_UNLIKELY(newCapacity > sMaxCapacity)) {
            if (reportFailure)
                this->reportAllocOverflow();
            return RehashFailed;
        }

        Entry* newTable = createTable(*this, newCapacity, reportFailure);
        if (!newTable)
            return RehashFailed;

        setTableSizeLog2(newLog2);
        removedCount = 0;
        gen++;
        table = newTable;

        Entry* end = oldTable + oldCap;
        for (Entry* src = oldTable; src < end; ++src) {
            if (src->isLive()) {
                HashNumber hn = src->getKeyHash();
                findFreeEntry(hn).setLive(
                    hn, mozilla::Move(const_cast<typename Entry::NonConstT&>(src->get())));
                src->destroy();
            }
        }

        this->free_(oldTable);
        return Rehashed;
    }

    bool shouldCompressTable()
    {
        return removedCount >= (capacity() >> 2);
    }

    RebuildStatus checkOverloaded(FailureBehavior reportFailure = ReportFailure)
    {
        if (!overloaded())
            return NotOverloaded;

        int deltaLog2;
        if (shouldCompressTable()) {
            METER(stats.compresses++);
            deltaLog2 = 0;
        } else {
            METER(stats.grows++);
            deltaLog2 = 1;
        }

        return changeTableSize(deltaLog2, reportFailure);
    }

    void checkOverRemoved()
    {
        if (overloaded()) {
            if (checkOverloaded(DontReportFailure) == RehashFailed)
                rehashTableInPlace();
        }
    }

    void remove(Entry& e)
    {
        MOZ_ASSERT(table);
        METER(stats.removes++);

        if (e.hasCollision()) {
            e.removeLive();
            removedCount++;
        } else {
            METER(stats.removeFrees++);
            e.clearLive();
        }
        entryCount--;
#ifdef JS_DEBUG
        mutationCount++;
#endif
    }

    void checkUnderloaded()
    {
        if (underloaded()) {
            METER(stats.shrinks++);
            (void) changeTableSize(-1, DontReportFailure);
        }
    }

    void compactIfUnderloaded()
    {
        int32_t resizeLog2 = 0;
        uint32_t newCapacity = capacity();
        while (wouldBeUnderloaded(newCapacity, entryCount)) {
            newCapacity = newCapacity >> 1;
            resizeLog2--;
        }

        if (resizeLog2 != 0)
            (void) changeTableSize(resizeLog2, DontReportFailure);
    }

    void rehashTableInPlace()
    {
        METER(stats.rehashes++);
        removedCount = 0;
        for (size_t i = 0; i < capacity(); ++i)
            table[i].unsetCollision();

        for (size_t i = 0; i < capacity();) {
            Entry* src = &table[i];

            if (!src->isLive() || src->hasCollision()) {
                ++i;
                continue;
            }

            HashNumber keyHash = src->getKeyHash();
            HashNumber h1 = hash1(keyHash);
            DoubleHash dh = hash2(keyHash);
            Entry* tgt = &table[h1];
            while (true) {
                if (!tgt->hasCollision()) {
                    src->swap(tgt);
                    tgt->setCollision();
                    break;
                }

                h1 = applyDoubleHash(h1, dh);
                tgt = &table[h1];
            }
        }

    }

    template <typename... Args>
    void putNewInfallibleInternal(const Lookup& l, Args&&... args)
    {
        MOZ_ASSERT(table);

        HashNumber keyHash = prepareHash(l);
        Entry* entry = &findFreeEntry(keyHash);
        MOZ_ASSERT(entry);

        if (entry->isRemoved()) {
            METER(stats.addOverRemoved++);
            removedCount--;
            keyHash |= sCollisionBit;
        }

        entry->setLive(keyHash, mozilla::Forward<Args>(args)...);
        entryCount++;
#ifdef JS_DEBUG
        mutationCount++;
#endif
    }

  public:
    void clear()
    {
        if (mozilla::IsPod<Entry>::value) {
            memset(table, 0, sizeof(*table) * capacity());
        } else {
            uint32_t tableCapacity = capacity();
            Entry* end = table + tableCapacity;
            for (Entry* e = table; e < end; ++e)
                e->clear();
        }
        removedCount = 0;
        entryCount = 0;
#ifdef JS_DEBUG
        mutationCount++;
#endif
    }

    void finish()
    {
#ifdef JS_DEBUG
        MOZ_ASSERT(!mEntered);
#endif
        if (!table)
            return;

        destroyTable(*this, table, capacity());
        table = nullptr;
        gen++;
        entryCount = 0;
        removedCount = 0;
#ifdef JS_DEBUG
        mutationCount++;
#endif
    }

    Range all() const
    {
        MOZ_ASSERT(table);
        return Range(*this, table, table + capacity());
    }

    bool empty() const
    {
        MOZ_ASSERT(table);
        return !entryCount;
    }

    uint32_t count() const
    {
        MOZ_ASSERT(table);
        return entryCount;
    }

    uint32_t capacity() const
    {
        MOZ_ASSERT(table);
        return JS_BIT(sHashBits - hashShift);
    }

    Generation generation() const
    {
        MOZ_ASSERT(table);
        return Generation(gen);
    }

    size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) const
    {
        return mallocSizeOf(table);
    }

    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const
    {
        return mallocSizeOf(this) + sizeOfExcludingThis(mallocSizeOf);
    }

    Ptr lookup(const Lookup& l) const
    {
        mozilla::ReentrancyGuard g(*this);
        if (!HasHash<HashPolicy>(l))
            return Ptr();
        HashNumber keyHash = prepareHash(l);
        return Ptr(lookup(l, keyHash, 0), *this);
    }

    Ptr readonlyThreadsafeLookup(const Lookup& l) const
    {
        if (!HasHash<HashPolicy>(l))
            return Ptr();
        HashNumber keyHash = prepareHash(l);
        return Ptr(lookup(l, keyHash, 0), *this);
    }

    AddPtr lookupForAdd(const Lookup& l) const
    {
        mozilla::ReentrancyGuard g(*this);
        if (!EnsureHash<HashPolicy>(l))
            return AddPtr();
        HashNumber keyHash = prepareHash(l);
        Entry& entry = lookup(l, keyHash, sCollisionBit);
        AddPtr p(entry, *this, keyHash);
        return p;
    }

    template <typename... Args>
    MOZ_MUST_USE bool add(AddPtr& p, Args&&... args)
    {
        mozilla::ReentrancyGuard g(*this);
        MOZ_ASSERT(table);
        MOZ_ASSERT(!p.found());
        MOZ_ASSERT(!(p.keyHash & sCollisionBit));

        if (p.isValid())
            return false;

        if (p.entry_->isRemoved()) {
            if (!this->checkSimulatedOOM())
                return false;
            METER(stats.addOverRemoved++);
            removedCount--;
            p.keyHash |= sCollisionBit;
        } else {
            RebuildStatus status = checkOverloaded();
            if (status == RehashFailed)
                return false;
            if (status == NotOverloaded && !this->checkSimulatedOOM())
                return false;
            if (status == Rehashed)
                p.entry_ = &findFreeEntry(p.keyHash);
        }

        p.entry_->setLive(p.keyHash, mozilla::Forward<Args>(args)...);
        entryCount++;
#ifdef JS_DEBUG
        mutationCount++;
        p.generation = generation();
        p.mutationCount = mutationCount;
#endif
        return true;
    }

    template <typename... Args>
    void putNewInfallible(const Lookup& l, Args&&... args)
    {
        MOZ_ASSERT(!lookup(l).found());
        mozilla::ReentrancyGuard g(*this);
        putNewInfallibleInternal(l, mozilla::Forward<Args>(args)...);
    }

    template <typename... Args>
    MOZ_MUST_USE bool putNew(const Lookup& l, Args&&... args)
    {
        if (!this->checkSimulatedOOM())
            return false;

        if (!EnsureHash<HashPolicy>(l))
            return false;

        if (checkOverloaded() == RehashFailed)
            return false;

        putNewInfallible(l, mozilla::Forward<Args>(args)...);
        return true;
    }

    template <typename... Args>
    MOZ_MUST_USE bool relookupOrAdd(AddPtr& p, const Lookup& l, Args&&... args)
    {
        if (p.isValid())
            return false;

#ifdef JS_DEBUG
        p.generation = generation();
        p.mutationCount = mutationCount;
#endif
        {
            mozilla::ReentrancyGuard g(*this);
            MOZ_ASSERT(prepareHash(l) == p.keyHash); // l has not been destroyed
            p.entry_ = &lookup(l, p.keyHash, sCollisionBit);
        }
        return p.found() || add(p, mozilla::Forward<Args>(args)...);
    }

    void remove(Ptr p)
    {
        MOZ_ASSERT(table);
        mozilla::ReentrancyGuard g(*this);
        MOZ_ASSERT(p.found());
        remove(*p.entry_);
        checkUnderloaded();
    }

    void rekeyWithoutRehash(Ptr p, const Lookup& l, const Key& k)
    {
        MOZ_ASSERT(table);
        mozilla::ReentrancyGuard g(*this);
        MOZ_ASSERT(p.found());
        typename HashTableEntry<T>::NonConstT t(mozilla::Move(*p));
        HashPolicy::setKey(t, const_cast<Key&>(k));
        remove(*p.entry_);
        putNewInfallibleInternal(l, mozilla::Move(t));
    }

    void rekeyAndMaybeRehash(Ptr p, const Lookup& l, const Key& k)
    {
        rekeyWithoutRehash(p, l, k);
        checkOverRemoved();
    }

#undef METER
};

} // namespace detail
} // namespace js

#endif  /* js_HashTable_h */

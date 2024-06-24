/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GCHashTable_h
#define GCHashTable_h

#include "js/GCPolicyAPI.h"
#include "js/HashTable.h"
#include "js/RootingAPI.h"
#include "js/SweepingAPI.h"
#include "js/TracingAPI.h"

namespace JS {

template <typename Key, typename Value>
struct DefaultMapSweepPolicy {
    static bool needsSweep(Key* key, Value* value) {
        return GCPolicy<Key>::needsSweep(key) || GCPolicy<Value>::needsSweep(value);
    }
};

template <typename Key,
          typename Value,
          typename HashPolicy = js::DefaultHasher<Key>,
          typename AllocPolicy = js::TempAllocPolicy,
          typename MapSweepPolicy = DefaultMapSweepPolicy<Key, Value>>
class GCHashMap : public js::HashMap<Key, Value, HashPolicy, AllocPolicy>
{
    using Base = js::HashMap<Key, Value, HashPolicy, AllocPolicy>;

  public:
    explicit GCHashMap(AllocPolicy a = AllocPolicy()) : Base(a)  {}

    static void trace(GCHashMap* map, JSTracer* trc) { map->trace(trc); }
    void trace(JSTracer* trc) {
        if (!this->initialized())
            return;
        for (typename Base::Enum e(*this); !e.empty(); e.popFront()) {
            GCPolicy<Value>::trace(trc, &e.front().value(), "hashmap value");
            GCPolicy<Key>::trace(trc, &e.front().mutableKey(), "hashmap key");
        }
    }

    void sweep() {
        if (!this->initialized())
            return;

        for (typename Base::Enum e(*this); !e.empty(); e.popFront()) {
            if (MapSweepPolicy::needsSweep(&e.front().mutableKey(), &e.front().value()))
                e.removeFront();
        }
    }

    GCHashMap(GCHashMap&& rhs) : Base(mozilla::Move(rhs)) {}
    void operator=(GCHashMap&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        Base::operator=(mozilla::Move(rhs));
    }

  private:
    GCHashMap(const GCHashMap& hm) = delete;
    GCHashMap& operator=(const GCHashMap& hm) = delete;
};

} // namespace JS

namespace js {

template <typename Key,
          typename Value,
          typename HashPolicy = DefaultHasher<Key>,
          typename AllocPolicy = TempAllocPolicy,
          typename MapSweepPolicy = JS::DefaultMapSweepPolicy<Key, Value>>
class GCRekeyableHashMap : public JS::GCHashMap<Key, Value, HashPolicy, AllocPolicy, MapSweepPolicy>
{
    using Base = JS::GCHashMap<Key, Value, HashPolicy, AllocPolicy>;

  public:
    explicit GCRekeyableHashMap(AllocPolicy a = AllocPolicy()) : Base(a)  {}

    void sweep() {
        if (!this->initialized())
            return;

        for (typename Base::Enum e(*this); !e.empty(); e.popFront()) {
            Key key(e.front().key());
            if (MapSweepPolicy::needsSweep(&key, &e.front().value()))
                e.removeFront();
            else if (!HashPolicy::match(key, e.front().key()))
                e.rekeyFront(key);
        }
    }

    GCRekeyableHashMap(GCRekeyableHashMap&& rhs) : Base(mozilla::Move(rhs)) {}
    void operator=(GCRekeyableHashMap&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        Base::operator=(mozilla::Move(rhs));
    }
};

template <typename Outer, typename... Args>
class GCHashMapOperations
{
    using Map = JS::GCHashMap<Args...>;
    using Lookup = typename Map::Lookup;

    const Map& map() const { return static_cast<const Outer*>(this)->get(); }

  public:
    using AddPtr = typename Map::AddPtr;
    using Ptr = typename Map::Ptr;
    using Range = typename Map::Range;

    bool initialized() const                   { return map().initialized(); }
    Ptr lookup(const Lookup& l) const          { return map().lookup(l); }
    AddPtr lookupForAdd(const Lookup& l) const { return map().lookupForAdd(l); }
    Range all() const                          { return map().all(); }
    bool empty() const                         { return map().empty(); }
    uint32_t count() const                     { return map().count(); }
    size_t capacity() const                    { return map().capacity(); }
    bool has(const Lookup& l) const            { return map().lookup(l).found(); }
    size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return map().sizeOfExcludingThis(mallocSizeOf);
    }
    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this) + map().sizeOfExcludingThis(mallocSizeOf);
    }
};

template <typename Outer, typename... Args>
class MutableGCHashMapOperations
  : public GCHashMapOperations<Outer, Args...>
{
    using Map = JS::GCHashMap<Args...>;
    using Lookup = typename Map::Lookup;

    Map& map() { return static_cast<Outer*>(this)->get(); }

  public:
    using AddPtr = typename Map::AddPtr;
    struct Enum : public Map::Enum { explicit Enum(Outer& o) : Map::Enum(o.map()) {} };
    using Ptr = typename Map::Ptr;
    using Range = typename Map::Range;

    bool init(uint32_t len = 16) { return map().init(len); }
    void clear()                 { map().clear(); }
    void finish()                { map().finish(); }
    void remove(Ptr p)           { map().remove(p); }

    template<typename KeyInput, typename ValueInput>
    bool add(AddPtr& p, KeyInput&& k, ValueInput&& v) {
        return map().add(p, mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput>
    bool add(AddPtr& p, KeyInput&& k) {
        return map().add(p, mozilla::Forward<KeyInput>(k), Map::Value());
    }

    template<typename KeyInput, typename ValueInput>
    bool relookupOrAdd(AddPtr& p, KeyInput&& k, ValueInput&& v) {
        return map().relookupOrAdd(p, k,
                                   mozilla::Forward<KeyInput>(k),
                                   mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput, typename ValueInput>
    bool put(KeyInput&& k, ValueInput&& v) {
        return map().put(mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }

    template<typename KeyInput, typename ValueInput>
    bool putNew(KeyInput&& k, ValueInput&& v) {
        return map().putNew(mozilla::Forward<KeyInput>(k), mozilla::Forward<ValueInput>(v));
    }
};

template <typename A, typename B, typename C, typename D, typename E>
class RootedBase<JS::GCHashMap<A,B,C,D,E>>
  : public MutableGCHashMapOperations<JS::Rooted<JS::GCHashMap<A,B,C,D,E>>, A,B,C,D,E>
{};

template <typename A, typename B, typename C, typename D, typename E>
class MutableHandleBase<JS::GCHashMap<A,B,C,D,E>>
  : public MutableGCHashMapOperations<JS::MutableHandle<JS::GCHashMap<A,B,C,D,E>>, A,B,C,D,E>
{};

template <typename A, typename B, typename C, typename D, typename E>
class HandleBase<JS::GCHashMap<A,B,C,D,E>>
  : public GCHashMapOperations<JS::Handle<JS::GCHashMap<A,B,C,D,E>>, A,B,C,D,E>
{};

template <typename A, typename B, typename C, typename D, typename E>
class WeakCacheBase<JS::GCHashMap<A,B,C,D,E>>
  : public MutableGCHashMapOperations<JS::WeakCache<JS::GCHashMap<A,B,C,D,E>>, A,B,C,D,E>
{};

} // namespace js

namespace JS {

template <typename T,
          typename HashPolicy = js::DefaultHasher<T>,
          typename AllocPolicy = js::TempAllocPolicy>
class GCHashSet : public js::HashSet<T, HashPolicy, AllocPolicy>
{
    using Base = js::HashSet<T, HashPolicy, AllocPolicy>;

  public:
    explicit GCHashSet(AllocPolicy a = AllocPolicy()) : Base(a)  {}

    static void trace(GCHashSet* set, JSTracer* trc) { set->trace(trc); }
    void trace(JSTracer* trc) {
        if (!this->initialized())
            return;
        for (typename Base::Enum e(*this); !e.empty(); e.popFront())
            GCPolicy<T>::trace(trc, &e.mutableFront(), "hashset element");
    }

    void sweep() {
        if (!this->initialized())
            return;
        for (typename Base::Enum e(*this); !e.empty(); e.popFront()) {
            if (GCPolicy<T>::needsSweep(&e.mutableFront()))
                e.removeFront();
        }
    }

    GCHashSet(GCHashSet&& rhs) : Base(mozilla::Move(rhs)) {}
    void operator=(GCHashSet&& rhs) {
        MOZ_ASSERT(this != &rhs, "self-move assignment is prohibited");
        Base::operator=(mozilla::Move(rhs));
    }

  private:
    GCHashSet(const GCHashSet& hs) = delete;
    GCHashSet& operator=(const GCHashSet& hs) = delete;
};

} // namespace JS

namespace js {

template <typename Outer, typename... Args>
class GCHashSetOperations
{
    using Set = JS::GCHashSet<Args...>;
    using Lookup = typename Set::Lookup;

    const Set& set() const { return static_cast<const Outer*>(this)->get(); }

  public:
    using AddPtr = typename Set::AddPtr;
    using Entry = typename Set::Entry;
    using Ptr = typename Set::Ptr;
    using Range = typename Set::Range;

    bool initialized() const                   { return set().initialized(); }
    Ptr lookup(const Lookup& l) const          { return set().lookup(l); }
    AddPtr lookupForAdd(const Lookup& l) const { return set().lookupForAdd(l); }
    Range all() const                          { return set().all(); }
    bool empty() const                         { return set().empty(); }
    uint32_t count() const                     { return set().count(); }
    size_t capacity() const                    { return set().capacity(); }
    bool has(const Lookup& l) const            { return set().lookup(l).found(); }
    size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return set().sizeOfExcludingThis(mallocSizeOf);
    }
    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this) + set().sizeOfExcludingThis(mallocSizeOf);
    }
};

template <typename Outer, typename... Args>
class MutableGCHashSetOperations
  : public GCHashSetOperations<Outer, Args...>
{
    using Set = JS::GCHashSet<Args...>;
    using Lookup = typename Set::Lookup;

    Set& set() { return static_cast<Outer*>(this)->get(); }

  public:
    using AddPtr = typename Set::AddPtr;
    using Entry = typename Set::Entry;
    struct Enum : public Set::Enum { explicit Enum(Outer& o) : Set::Enum(o.set()) {} };
    using Ptr = typename Set::Ptr;
    using Range = typename Set::Range;

    bool init(uint32_t len = 16) { return set().init(len); }
    void clear()                 { set().clear(); }
    void finish()                { set().finish(); }
    void remove(Ptr p)           { set().remove(p); }
    void remove(const Lookup& l) { set().remove(l); }

    template<typename TInput>
    bool add(AddPtr& p, TInput&& t) {
        return set().add(p, mozilla::Forward<TInput>(t));
    }

    template<typename TInput>
    bool relookupOrAdd(AddPtr& p, const Lookup& l, TInput&& t) {
        return set().relookupOrAdd(p, l, mozilla::Forward<TInput>(t));
    }

    template<typename TInput>
    bool put(TInput&& t) {
        return set().put(mozilla::Forward<TInput>(t));
    }

    template<typename TInput>
    bool putNew(TInput&& t) {
        return set().putNew(mozilla::Forward<TInput>(t));
    }

    template<typename TInput>
    bool putNew(const Lookup& l, TInput&& t) {
        return set().putNew(l, mozilla::Forward<TInput>(t));
    }
};

template <typename T, typename HP, typename AP>
class RootedBase<JS::GCHashSet<T, HP, AP>>
  : public MutableGCHashSetOperations<JS::Rooted<JS::GCHashSet<T, HP, AP>>, T, HP, AP>
{
};

template <typename T, typename HP, typename AP>
class MutableHandleBase<JS::GCHashSet<T, HP, AP>>
  : public MutableGCHashSetOperations<JS::MutableHandle<JS::GCHashSet<T, HP, AP>>, T, HP, AP>
{
};

template <typename T, typename HP, typename AP>
class HandleBase<JS::GCHashSet<T, HP, AP>>
  : public GCHashSetOperations<JS::Handle<JS::GCHashSet<T, HP, AP>>, T, HP, AP>
{
};

template <typename T, typename HP, typename AP>
class WeakCacheBase<JS::GCHashSet<T, HP, AP>>
  : public MutableGCHashSetOperations<JS::WeakCache<JS::GCHashSet<T, HP, AP>>, T, HP, AP>
{
};

} /* namespace js */

#endif /* GCHashTable_h */

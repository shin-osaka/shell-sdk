/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UbiNodeCensus_h
#define js_UbiNodeCensus_h

#include "mozilla/Attributes.h"
#include "mozilla/Move.h"

#include <algorithm>

#include "jsapi.h"

#include "js/UbiNode.h"
#include "js/UbiNodeBreadthFirst.h"



namespace JS {
namespace ubi {

struct Census;

class CountBase;

struct CountDeleter {
    void operator()(CountBase*);
};

using CountBasePtr = js::UniquePtr<CountBase, CountDeleter>;

struct CountType {
    explicit CountType() { }
    virtual ~CountType() { }

    virtual void destructCount(CountBase& count) = 0;

    virtual CountBasePtr makeCount() = 0;

    virtual void traceCount(CountBase& count, JSTracer* trc) = 0;

    virtual MOZ_MUST_USE bool count(CountBase& count,
                                    mozilla::MallocSizeOf mallocSizeOf,
                                    const Node& node) = 0;

    virtual MOZ_MUST_USE bool report(JSContext* cx, CountBase& count,
                                     MutableHandleValue report) = 0;
};

using CountTypePtr = js::UniquePtr<CountType>;

class CountBase {
    CountType& type;

  protected:
    ~CountBase() { }

  public:
    explicit CountBase(CountType& type)
      : type(type)
      , total_(0)
      , smallestNodeIdCounted_(SIZE_MAX)
    { }

    MOZ_MUST_USE bool count(mozilla::MallocSizeOf mallocSizeOf, const Node& node) {
        total_++;

        auto id = node.identifier();
        if (id < smallestNodeIdCounted_) {
            smallestNodeIdCounted_ = id;
        }

#ifdef DEBUG
        size_t oldTotal = total_;
#endif

        bool ret = type.count(*this, mallocSizeOf, node);

        MOZ_ASSERT(total_ == oldTotal,
                   "CountType::count should not increment total_, CountBase::count handles that");

        return ret;
    }

    MOZ_MUST_USE bool report(JSContext* cx, MutableHandleValue report) {
        return type.report(cx, *this, report);
    }

    void destruct() { return type.destructCount(*this); }

    void trace(JSTracer* trc) { type.traceCount(*this, trc); }

    size_t total_;

    Node::Id smallestNodeIdCounted_;
};

class RootedCount : JS::CustomAutoRooter {
    CountBasePtr count;

    void trace(JSTracer* trc) override { count->trace(trc); }

  public:
    RootedCount(JSContext* cx, CountBasePtr&& count)
        : CustomAutoRooter(cx),
          count(Move(count))
          { }
    CountBase* operator->() const { return count.get(); }
    explicit operator bool() const { return count.get(); }
    operator CountBasePtr&() { return count; }
};

struct Census {
    JSContext* const cx;
    JS::ZoneSet targetZones;
    Zone* atomsZone;

    explicit Census(JSContext* cx) : cx(cx), atomsZone(nullptr) { }

    MOZ_MUST_USE bool init();
};

class CensusHandler {
    Census& census;
    CountBasePtr& rootCount;
    mozilla::MallocSizeOf mallocSizeOf;

  public:
    CensusHandler(Census& census, CountBasePtr& rootCount, mozilla::MallocSizeOf mallocSizeOf)
      : census(census),
        rootCount(rootCount),
        mallocSizeOf(mallocSizeOf)
    { }

    MOZ_MUST_USE bool report(JSContext* cx, MutableHandleValue report) {
        return rootCount->report(cx, report);
    }

    class NodeData { };

    MOZ_MUST_USE bool operator() (BreadthFirst<CensusHandler>& traversal,
                                  Node origin, const Edge& edge,
                                  NodeData* referentData, bool first);
};

using CensusTraversal = BreadthFirst<CensusHandler>;

MOZ_MUST_USE bool ParseCensusOptions(JSContext* cx, Census& census, HandleObject options,
                                     CountTypePtr& outResult);

CountTypePtr ParseBreakdown(JSContext* cx, HandleValue breakdownValue);


} // namespace ubi
} // namespace JS

#endif // js_UbiNodeCensus_h

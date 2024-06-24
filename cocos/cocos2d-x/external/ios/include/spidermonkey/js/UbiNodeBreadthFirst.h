/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UbiNodeBreadthFirst_h
#define js_UbiNodeBreadthFirst_h

#include "js/UbiNode.h"
#include "js/Utility.h"
#include "js/Vector.h"

namespace JS {
namespace ubi {

template<typename Handler>
struct BreadthFirst {

    BreadthFirst(JSContext* cx, Handler& handler, const JS::AutoCheckCannotGC& noGC)
      : wantNames(true), cx(cx), visited(), handler(handler), pending(),
        traversalBegun(false), stopRequested(false), abandonRequested(false)
    { }

    bool init() { return visited.init(); }

    bool addStart(Node node) { return pending.append(node); }

    bool addStartVisited(Node node) {
        typename NodeMap::AddPtr ptr = visited.lookupForAdd(node);
        if (!ptr && !visited.add(ptr, node, typename Handler::NodeData()))
            return false;
        return addStart(node);
    }

    bool wantNames;

    bool traverse()
    {
        MOZ_ASSERT(!traversalBegun);
        traversalBegun = true;

        while (!pending.empty()) {
            Node origin = pending.front();
            pending.popFront();

            auto range = origin.edges(cx, wantNames);
            if (!range)
                return false;

            for (; !range->empty(); range->popFront()) {
                MOZ_ASSERT(!stopRequested);

                Edge& edge = range->front();
                typename NodeMap::AddPtr a = visited.lookupForAdd(edge.referent);
                bool first = !a;

                if (first) {
                    if (!visited.add(a, edge.referent, typename Handler::NodeData()))
                        return false;
                }

                MOZ_ASSERT(a);

                if (!handler(*this, origin, edge, &a->value(), first))
                    return false;

                if (stopRequested)
                    return true;

                if (abandonRequested) {
                    abandonRequested = false;
                } else if (first) {
                    if (!pending.append(edge.referent))
                        return false;
                }
            }
        }

        return true;
    }

    void stop() { stopRequested = true; }

    void abandonReferent() { abandonRequested = true; }

    JSContext* cx;

    using NodeMap = js::HashMap<Node, typename Handler::NodeData, js::DefaultHasher<Node>,
                                js::SystemAllocPolicy>;
    NodeMap visited;

  private:
    Handler& handler;

    template <typename T>
    class Queue {
        js::Vector<T, 0, js::SystemAllocPolicy> head, tail;
        size_t frontIndex;
      public:
        Queue() : head(), tail(), frontIndex(0) { }
        bool empty() { return frontIndex >= head.length(); }
        T& front() {
            MOZ_ASSERT(!empty());
            return head[frontIndex];
        }
        void popFront() {
            MOZ_ASSERT(!empty());
            frontIndex++;
            if (frontIndex >= head.length()) {
                head.clearAndFree();
                head.swap(tail);
                frontIndex = 0;
            }
        }
        bool append(const T& elt) {
            return frontIndex == 0 ? head.append(elt) : tail.append(elt);
        }
    };

    Queue<Node> pending;

    bool traversalBegun;

    bool stopRequested;

    bool abandonRequested;
};

} // namespace ubi
} // namespace JS

#endif // js_UbiNodeBreadthFirst_h

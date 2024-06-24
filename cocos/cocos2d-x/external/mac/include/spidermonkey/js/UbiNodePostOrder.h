/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UbiNodePostOrder_h
#define js_UbiNodePostOrder_h

#include "mozilla/Attributes.h"
#include "mozilla/Maybe.h"
#include "mozilla/Move.h"

#include "jsalloc.h"

#include "js/UbiNode.h"
#include "js/Utility.h"
#include "js/Vector.h"

namespace JS {
namespace ubi {

/**
 * A post-order depth-first traversal of `ubi::Node` graphs.
 *
 * No GC may occur while an instance of `PostOrder` is live.
 *
 * The `NodeVisitor` type provided to `PostOrder::traverse` must have the
 * following member:
 *
 *   bool operator()(Node& node)
 *
 *     The node visitor method. This method is called once for each `node`
 *     reachable from the start set in post-order.
 *
 *     This visitor function should return true on success, or false if an error
 *     occurs. A false return value terminates the traversal immediately, and
 *     causes `PostOrder::traverse` to return false.
 *
 * The `EdgeVisitor` type provided to `PostOrder::traverse` must have the
 * following member:
 *
 *   bool operator()(Node& origin, Edge& edge)
 *
 *     The edge visitor method. This method is called once for each outgoing
 *     `edge` from `origin` that is reachable from the start set.
 *
 *     NB: UNLIKE NODES, THERE IS NO GUARANTEED ORDER IN WHICH EDGES AND THEIR
 *     ORIGINS ARE VISITED!
 *
 *     This visitor function should return true on success, or false if an error
 *     occurs. A false return value terminates the traversal immediately, and
 *     causes `PostOrder::traverse` to return false.
 */
struct PostOrder {
  private:
    struct OriginAndEdges {
        Node                 origin;
        EdgeVector           edges;

        OriginAndEdges(const Node& node, EdgeVector&& edges)
          : origin(node)
          , edges(mozilla::Move(edges))
        { }

        OriginAndEdges(const OriginAndEdges& rhs) = delete;
        OriginAndEdges& operator=(const OriginAndEdges& rhs) = delete;

        OriginAndEdges(OriginAndEdges&& rhs)
          : origin(rhs.origin)
          , edges(mozilla::Move(rhs.edges))
        {
            MOZ_ASSERT(&rhs != this, "self-move disallowed");
        }

        OriginAndEdges& operator=(OriginAndEdges&& rhs) {
            this->~OriginAndEdges();
            new (this) OriginAndEdges(mozilla::Move(rhs));
            return *this;
        }
    };

    using Stack = js::Vector<OriginAndEdges, 256, js::SystemAllocPolicy>;
    using Set = js::HashSet<Node, js::DefaultHasher<Node>, js::SystemAllocPolicy>;

    JSContext*               cx;
    Set                      seen;
    Stack                    stack;
#ifdef DEBUG
    bool                     traversed;
#endif

  private:
    MOZ_MUST_USE bool fillEdgesFromRange(EdgeVector& edges, js::UniquePtr<EdgeRange>& range) {
        MOZ_ASSERT(range);
        for ( ; !range->empty(); range->popFront()) {
            if (!edges.append(mozilla::Move(range->front())))
                return false;
        }
        return true;
    }

    MOZ_MUST_USE bool pushForTraversing(const Node& node) {
        EdgeVector edges;
        auto range = node.edges(cx, /* wantNames */ false);
        return range &&
            fillEdgesFromRange(edges, range) &&
            stack.append(OriginAndEdges(node, mozilla::Move(edges)));
    }


  public:
    PostOrder(JSContext* cx, AutoCheckCannotGC&)
      : cx(cx)
      , seen()
      , stack()
#ifdef DEBUG
      , traversed(false)
#endif
    { }

    MOZ_MUST_USE bool init() { return seen.init(); }

    MOZ_MUST_USE bool addStart(const Node& node) {
        if (!seen.put(node))
            return false;
        return pushForTraversing(node);
    }

    template<typename NodeVisitor, typename EdgeVisitor>
    MOZ_MUST_USE bool traverse(NodeVisitor onNode, EdgeVisitor onEdge) {
#ifdef DEBUG
        MOZ_ASSERT(!traversed, "Can only traverse() once!");
        traversed = true;
#endif

        while (!stack.empty()) {
            auto& origin = stack.back().origin;
            auto& edges = stack.back().edges;

            if (edges.empty()) {
                if (!onNode(origin))
                    return false;
                stack.popBack();
                continue;
            }

            Edge edge = mozilla::Move(edges.back());
            edges.popBack();

            if (!onEdge(origin, edge))
                return false;

            auto ptr = seen.lookupForAdd(edge.referent);
            if (ptr)
                continue;

            if (!seen.add(ptr, edge.referent) ||
                !pushForTraversing(edge.referent))
            {
                return false;
            }
        }

        return true;
    }
};

} // namespace ubi
} // namespace JS

#endif // js_UbiNodePostOrder_h

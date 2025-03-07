/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UbiNodeShortestPaths_h
#define js_UbiNodeShortestPaths_h

#include "mozilla/Attributes.h"
#include "mozilla/Maybe.h"
#include "mozilla/Move.h"

#include "jsalloc.h"

#include "js/UbiNodeBreadthFirst.h"
#include "js/Vector.h"

namespace JS {
namespace ubi {

/**
 * A back edge along a path in the heap graph.
 */
struct JS_PUBLIC_API(BackEdge)
{
  private:
    Node predecessor_;
    EdgeName name_;

  public:
    using Ptr = mozilla::UniquePtr<BackEdge, JS::DeletePolicy<BackEdge>>;

    BackEdge() : predecessor_(), name_(nullptr) { }

    MOZ_MUST_USE bool init(const Node& predecessor, Edge& edge) {
        MOZ_ASSERT(!predecessor_);
        MOZ_ASSERT(!name_);

        predecessor_ = predecessor;
        name_ = mozilla::Move(edge.name);
        return true;
    }

    BackEdge(const BackEdge&) = delete;
    BackEdge& operator=(const BackEdge&) = delete;

    BackEdge(BackEdge&& rhs)
      : predecessor_(rhs.predecessor_)
      , name_(mozilla::Move(rhs.name_))
    {
        MOZ_ASSERT(&rhs != this);
    }

    BackEdge& operator=(BackEdge&& rhs) {
        this->~BackEdge();
        new(this) BackEdge(Move(rhs));
        return *this;
    }

    Ptr clone() const;

    const EdgeName& name() const { return name_; }
    EdgeName& name() { return name_; }

    const JS::ubi::Node& predecessor() const { return predecessor_; }
};

/**
 * A path is a series of back edges from which we discovered a target node.
 */
using Path = JS::ubi::Vector<BackEdge*>;

/**
 * The `JS::ubi::ShortestPaths` type represents a collection of up to N shortest
 * retaining paths for each of a target set of nodes, starting from the same
 * root node.
 */
struct JS_PUBLIC_API(ShortestPaths)
{
  private:

    using BackEdgeVector = JS::ubi::Vector<BackEdge::Ptr>;
    using NodeToBackEdgeVectorMap = js::HashMap<Node, BackEdgeVector, js::DefaultHasher<Node>,
                                                js::SystemAllocPolicy>;

    struct Handler;
    using Traversal = BreadthFirst<Handler>;

    /**
     * A `JS::ubi::BreadthFirst` traversal handler that records back edges for
     * how we reached each node, allowing us to reconstruct the shortest
     * retaining paths after the traversal.
     */
    struct Handler
    {
        using NodeData = BackEdge;

        ShortestPaths& shortestPaths;
        size_t totalMaxPathsToRecord;
        size_t totalPathsRecorded;

        explicit Handler(ShortestPaths& shortestPaths)
          : shortestPaths(shortestPaths)
          , totalMaxPathsToRecord(shortestPaths.targets_.count() * shortestPaths.maxNumPaths_)
          , totalPathsRecorded(0)
        {
        }

        bool
        operator()(Traversal& traversal, JS::ubi::Node origin, JS::ubi::Edge& edge,
                   BackEdge* back, bool first)
        {
            MOZ_ASSERT(back);
            MOZ_ASSERT(origin == shortestPaths.root_ || traversal.visited.has(origin));
            MOZ_ASSERT(totalPathsRecorded < totalMaxPathsToRecord);

            if (first && !back->init(origin, edge))
                return false;

            if (!shortestPaths.targets_.has(edge.referent))
                return true;


            if (first) {
                BackEdgeVector paths;
                if (!paths.reserve(shortestPaths.maxNumPaths_))
                    return false;
                auto cloned = back->clone();
                if (!cloned)
                    return false;
                paths.infallibleAppend(mozilla::Move(cloned));
                if (!shortestPaths.paths_.putNew(edge.referent, mozilla::Move(paths)))
                    return false;
                totalPathsRecorded++;
            } else {
                auto ptr = shortestPaths.paths_.lookup(edge.referent);
                MOZ_ASSERT(ptr,
                           "This isn't the first time we have seen the target node `edge.referent`. "
                           "We should have inserted it into shortestPaths.paths_ the first time we "
                           "saw it.");

                if (ptr->value().length() < shortestPaths.maxNumPaths_) {
                    BackEdge::Ptr thisBackEdge(js_new<BackEdge>());
                    if (!thisBackEdge || !thisBackEdge->init(origin, edge))
                        return false;
                    ptr->value().infallibleAppend(mozilla::Move(thisBackEdge));
                    totalPathsRecorded++;
                }
            }

            MOZ_ASSERT(totalPathsRecorded <= totalMaxPathsToRecord);
            if (totalPathsRecorded == totalMaxPathsToRecord)
                traversal.stop();

            return true;
        }

    };

    uint32_t maxNumPaths_;

    Node root_;

    NodeSet targets_;

    NodeToBackEdgeVectorMap paths_;

    Traversal::NodeMap backEdges_;

  private:

    ShortestPaths(uint32_t maxNumPaths, const Node& root, NodeSet&& targets)
      : maxNumPaths_(maxNumPaths)
      , root_(root)
      , targets_(mozilla::Move(targets))
      , paths_()
      , backEdges_()
    {
        MOZ_ASSERT(maxNumPaths_ > 0);
        MOZ_ASSERT(root_);
        MOZ_ASSERT(targets_.initialized());
    }

    bool initialized() const {
        return targets_.initialized() &&
               paths_.initialized() &&
               backEdges_.initialized();
    }

  public:

    ShortestPaths(ShortestPaths&& rhs)
      : maxNumPaths_(rhs.maxNumPaths_)
      , root_(rhs.root_)
      , targets_(mozilla::Move(rhs.targets_))
      , paths_(mozilla::Move(rhs.paths_))
      , backEdges_(mozilla::Move(rhs.backEdges_))
    {
        MOZ_ASSERT(this != &rhs, "self-move is not allowed");
    }

    ShortestPaths& operator=(ShortestPaths&& rhs) {
        this->~ShortestPaths();
        new (this) ShortestPaths(mozilla::Move(rhs));
        return *this;
    }

    ShortestPaths(const ShortestPaths&) = delete;
    ShortestPaths& operator=(const ShortestPaths&) = delete;

    /**
     * Construct a new `JS::ubi::ShortestPaths`, finding up to `maxNumPaths`
     * shortest retaining paths for each target node in `targets` starting from
     * `root`.
     *
     * The resulting `ShortestPaths` instance must not outlive the
     * `JS::ubi::Node` graph it was constructed from.
     *
     *   - For `JS::ubi::Node` graphs backed by the live heap graph, this means
     *     that the `ShortestPaths`'s lifetime _must_ be contained within the
     *     scope of the provided `AutoCheckCannotGC` reference because a GC will
     *     invalidate the nodes.
     *
     *   - For `JS::ubi::Node` graphs backed by some other offline structure
     *     provided by the embedder, the resulting `ShortestPaths`'s lifetime is
     *     bounded by that offline structure's lifetime.
     *
     * Returns `mozilla::Nothing()` on OOM failure. It is the caller's
     * responsibility to handle and report the OOM.
     */
    static mozilla::Maybe<ShortestPaths>
    Create(JSContext* cx, AutoCheckCannotGC& noGC, uint32_t maxNumPaths, const Node& root, NodeSet&& targets) {
        MOZ_ASSERT(targets.count() > 0);
        MOZ_ASSERT(maxNumPaths > 0);

        size_t count = targets.count();
        ShortestPaths paths(maxNumPaths, root, mozilla::Move(targets));
        if (!paths.paths_.init(count))
            return mozilla::Nothing();

        Handler handler(paths);
        Traversal traversal(cx, handler, noGC);
        traversal.wantNames = true;
        if (!traversal.init() || !traversal.addStart(root) || !traversal.traverse())
            return mozilla::Nothing();

        paths.backEdges_ = mozilla::Move(traversal.visited);

        MOZ_ASSERT(paths.initialized());
        return mozilla::Some(mozilla::Move(paths));
    }

    /**
     * Get a range that iterates over each target node we searched for retaining
     * paths for. The returned range must not outlive the `ShortestPaths`
     * instance.
     */
    NodeSet::Range eachTarget() const {
        MOZ_ASSERT(initialized());
        return targets_.all();
    }

    /**
     * Invoke the provided functor/lambda/callable once for each retaining path
     * discovered for `target`. The `func` is passed a single `JS::ubi::Path&`
     * argument, which contains each edge along the path ordered starting from
     * the root and ending at the target, and must not outlive the scope of the
     * call.
     *
     * Note that it is possible that we did not find any paths from the root to
     * the given target, in which case `func` will not be invoked.
     */
    template <class Func>
    MOZ_MUST_USE bool forEachPath(const Node& target, Func func) {
        MOZ_ASSERT(initialized());
        MOZ_ASSERT(targets_.has(target));

        auto ptr = paths_.lookup(target);

        if (!ptr)
            return true;

        MOZ_ASSERT(ptr->value().length() <= maxNumPaths_);

        Path path;
        for (const auto& backEdge : ptr->value()) {
            path.clear();

            if (!path.append(backEdge.get()))
                return false;

            Node here = backEdge->predecessor();
            MOZ_ASSERT(here);

            while (here != root_) {
                auto p = backEdges_.lookup(here);
                MOZ_ASSERT(p);
                if (!path.append(&p->value()))
                    return false;
                here = p->value().predecessor();
                MOZ_ASSERT(here);
            }

            path.reverse();

            if (!func(path))
                return false;
        }

        return true;
    }
};

#ifdef DEBUG
JS_PUBLIC_API(void)
dumpPaths(JSRuntime* rt, Node node, uint32_t maxNumPaths = 10);
#endif

} // namespace ubi
} // namespace JS

#endif // js_UbiNodeShortestPaths_h

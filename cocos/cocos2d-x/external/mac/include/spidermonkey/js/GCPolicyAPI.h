/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */



#ifndef GCPolicyAPI_h
#define GCPolicyAPI_h

#include "mozilla/UniquePtr.h"

#include "js/TraceKind.h"
#include "js/TracingAPI.h"

#define FOR_EACH_PUBLIC_GC_POINTER_TYPE(D) \
    D(JS::Symbol*) \
    D(JSAtom*) \
    D(JSFunction*) \
    D(JSObject*) \
    D(JSScript*) \
    D(JSString*)

#define FOR_EACH_PUBLIC_TAGGED_GC_POINTER_TYPE(D) \
    D(JS::Value) \
    D(jsid)

#define FOR_EACH_PUBLIC_AGGREGATE_GC_POINTER_TYPE(D) \
    D(JSPropertyDescriptor)

class JSAtom;
class JSFunction;
class JSObject;
class JSScript;
class JSString;
namespace JS {
class Symbol;
}

namespace JS {

template <typename T>
struct StructGCPolicy
{
    static T initial() {
        return T();
    }

    static void trace(JSTracer* trc, T* tp, const char* name) {
        tp->trace(trc);
    }

    static void sweep(T* tp) {
        return tp->sweep();
    }

    static bool needsSweep(T* tp) {
        return tp->needsSweep();
    }
};

template <typename T> struct GCPolicy : public StructGCPolicy<T> {};

template <typename T>
struct IgnoreGCPolicy {
    static T initial() { return T(); }
    static void trace(JSTracer* trc, T* t, const char* name) {}
    static bool needsSweep(T* v) { return false; }
};
template <> struct GCPolicy<uint32_t> : public IgnoreGCPolicy<uint32_t> {};
template <> struct GCPolicy<uint64_t> : public IgnoreGCPolicy<uint64_t> {};

template <typename T>
struct GCPointerPolicy
{
    static T initial() { return nullptr; }
    static void trace(JSTracer* trc, T* vp, const char* name) {
        if (*vp)
            js::UnsafeTraceManuallyBarrieredEdge(trc, vp, name);
    }
    static bool needsSweep(T* vp) {
        if (*vp)
            return js::gc::IsAboutToBeFinalizedUnbarriered(vp);
        return false;
    }
};
template <> struct GCPolicy<JS::Symbol*> : public GCPointerPolicy<JS::Symbol*> {};
template <> struct GCPolicy<JSAtom*> : public GCPointerPolicy<JSAtom*> {};
template <> struct GCPolicy<JSFunction*> : public GCPointerPolicy<JSFunction*> {};
template <> struct GCPolicy<JSObject*> : public GCPointerPolicy<JSObject*> {};
template <> struct GCPolicy<JSScript*> : public GCPointerPolicy<JSScript*> {};
template <> struct GCPolicy<JSString*> : public GCPointerPolicy<JSString*> {};

template <typename T>
struct GCPolicy<JS::Heap<T>>
{
    static void trace(JSTracer* trc, JS::Heap<T>* thingp, const char* name) {
        TraceEdge(trc, thingp, name);
    }
    static bool needsSweep(JS::Heap<T>* thingp) {
        return js::gc::EdgeNeedsSweep(thingp);
    }
};

template <typename T, typename D>
struct GCPolicy<mozilla::UniquePtr<T, D>>
{
    static mozilla::UniquePtr<T,D> initial() { return mozilla::UniquePtr<T,D>(); }
    static void trace(JSTracer* trc, mozilla::UniquePtr<T,D>* tp, const char* name) {
        if (tp->get())
            GCPolicy<T>::trace(trc, tp->get(), name);
    }
    static bool needsSweep(mozilla::UniquePtr<T,D>* tp) {
        if (tp->get())
            return GCPolicy<T>::needsSweep(tp->get());
        return false;
    }
};

} // namespace JS

#endif // GCPolicyAPI_h

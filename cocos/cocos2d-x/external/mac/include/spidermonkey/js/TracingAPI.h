/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_TracingAPI_h
#define js_TracingAPI_h

#include "jsalloc.h"

#include "js/HashTable.h"
#include "js/HeapAPI.h"
#include "js/TraceKind.h"

class JS_PUBLIC_API(JSTracer);

namespace JS {
class JS_PUBLIC_API(CallbackTracer);
template <typename T> class Heap;
template <typename T> class TenuredHeap;

/** Returns a static string equivalent of |kind|. */
JS_FRIEND_API(const char*)
GCTraceKindToAscii(JS::TraceKind kind);

} // namespace JS

enum WeakMapTraceKind {
    /**
     * Do not trace into weak map keys or values during traversal. Users must
     * handle weak maps manually.
     */
    DoNotTraceWeakMaps,

    /**
     * Do true ephemeron marking with a weak key lookup marking phase. This is
     * the default for GCMarker.
     */
    ExpandWeakMaps,

    /**
     * Trace through to all values, irrespective of whether the keys are live
     * or not. Used for non-marking tracers.
     */
    TraceWeakMapValues,

    /**
     * Trace through to all keys and values, irrespective of whether the keys
     * are live or not. Used for non-marking tracers.
     */
    TraceWeakMapKeysValues
};

class JS_PUBLIC_API(JSTracer)
{
  public:
    JSRuntime* runtime() const { return runtime_; }

    WeakMapTraceKind weakMapAction() const { return weakMapAction_; }

    enum class TracerKindTag {
        Marking,

        WeakMarking,

        Tenuring,

        Callback
    };
    bool isMarkingTracer() const { return tag_ == TracerKindTag::Marking || tag_ == TracerKindTag::WeakMarking; }
    bool isWeakMarkingTracer() const { return tag_ == TracerKindTag::WeakMarking; }
    bool isTenuringTracer() const { return tag_ == TracerKindTag::Tenuring; }
    bool isCallbackTracer() const { return tag_ == TracerKindTag::Callback; }
    inline JS::CallbackTracer* asCallbackTracer();
#ifdef DEBUG
    bool checkEdges() { return checkEdges_; }
#endif

  protected:
    JSTracer(JSRuntime* rt, TracerKindTag tag,
             WeakMapTraceKind weakTraceKind = TraceWeakMapValues)
      : runtime_(rt)
      , weakMapAction_(weakTraceKind)
#ifdef DEBUG
      , checkEdges_(true)
#endif
      , tag_(tag)
    {}

#ifdef DEBUG
    void setCheckEdges(bool check) {
        checkEdges_ = check;
    }
#endif

  private:
    JSRuntime* runtime_;
    WeakMapTraceKind weakMapAction_;
#ifdef DEBUG
    bool checkEdges_;
#endif

  protected:
    TracerKindTag tag_;
};

namespace JS {

class AutoTracingName;
class AutoTracingIndex;
class AutoTracingCallback;

class JS_PUBLIC_API(CallbackTracer) : public JSTracer
{
  public:
    CallbackTracer(JSRuntime* rt, WeakMapTraceKind weakTraceKind = TraceWeakMapValues)
      : JSTracer(rt, JSTracer::TracerKindTag::Callback, weakTraceKind),
        contextName_(nullptr), contextIndex_(InvalidIndex), contextFunctor_(nullptr)
    {}
    CallbackTracer(JSContext* cx, WeakMapTraceKind weakTraceKind = TraceWeakMapValues);

    virtual void onObjectEdge(JSObject** objp) { onChild(JS::GCCellPtr(*objp)); }
    virtual void onStringEdge(JSString** strp) { onChild(JS::GCCellPtr(*strp)); }
    virtual void onSymbolEdge(JS::Symbol** symp) { onChild(JS::GCCellPtr(*symp)); }
    virtual void onScriptEdge(JSScript** scriptp) { onChild(JS::GCCellPtr(*scriptp)); }
    virtual void onShapeEdge(js::Shape** shapep) {
        onChild(JS::GCCellPtr(*shapep, JS::TraceKind::Shape));
    }
    virtual void onObjectGroupEdge(js::ObjectGroup** groupp) {
        onChild(JS::GCCellPtr(*groupp, JS::TraceKind::ObjectGroup));
    }
    virtual void onBaseShapeEdge(js::BaseShape** basep) {
        onChild(JS::GCCellPtr(*basep, JS::TraceKind::BaseShape));
    }
    virtual void onJitCodeEdge(js::jit::JitCode** codep) {
        onChild(JS::GCCellPtr(*codep, JS::TraceKind::JitCode));
    }
    virtual void onLazyScriptEdge(js::LazyScript** lazyp) {
        onChild(JS::GCCellPtr(*lazyp, JS::TraceKind::LazyScript));
    }
    virtual void onScopeEdge(js::Scope** scopep) {
        onChild(JS::GCCellPtr(*scopep, JS::TraceKind::Scope));
    }

    virtual void onChild(const JS::GCCellPtr& thing) = 0;


    const char* contextName() const { MOZ_ASSERT(contextName_); return contextName_; }

    const static size_t InvalidIndex = size_t(-1);
    size_t contextIndex() const { return contextIndex_; }

    void getTracingEdgeName(char* buffer, size_t bufferSize);

    class ContextFunctor {
      public:
        virtual void operator()(CallbackTracer* trc, char* buf, size_t bufsize) = 0;
    };

#ifdef DEBUG
    enum class TracerKind { DoNotCare, Moving, GrayBuffering, VerifyTraceProtoAndIface };
    virtual TracerKind getTracerKind() const { return TracerKind::DoNotCare; }
#endif

    void dispatchToOnEdge(JSObject** objp) { onObjectEdge(objp); }
    void dispatchToOnEdge(JSString** strp) { onStringEdge(strp); }
    void dispatchToOnEdge(JS::Symbol** symp) { onSymbolEdge(symp); }
    void dispatchToOnEdge(JSScript** scriptp) { onScriptEdge(scriptp); }
    void dispatchToOnEdge(js::Shape** shapep) { onShapeEdge(shapep); }
    void dispatchToOnEdge(js::ObjectGroup** groupp) { onObjectGroupEdge(groupp); }
    void dispatchToOnEdge(js::BaseShape** basep) { onBaseShapeEdge(basep); }
    void dispatchToOnEdge(js::jit::JitCode** codep) { onJitCodeEdge(codep); }
    void dispatchToOnEdge(js::LazyScript** lazyp) { onLazyScriptEdge(lazyp); }
    void dispatchToOnEdge(js::Scope** scopep) { onScopeEdge(scopep); }

  private:
    friend class AutoTracingName;
    const char* contextName_;

    friend class AutoTracingIndex;
    size_t contextIndex_;

    friend class AutoTracingDetails;
    ContextFunctor* contextFunctor_;
};

class MOZ_RAII AutoTracingName
{
    CallbackTracer* trc_;
    const char* prior_;

  public:
    AutoTracingName(CallbackTracer* trc, const char* name) : trc_(trc), prior_(trc->contextName_) {
        MOZ_ASSERT(name);
        trc->contextName_ = name;
    }
    ~AutoTracingName() {
        MOZ_ASSERT(trc_->contextName_);
        trc_->contextName_ = prior_;
    }
};

class MOZ_RAII AutoTracingIndex
{
    CallbackTracer* trc_;

  public:
    explicit AutoTracingIndex(JSTracer* trc, size_t initial = 0) : trc_(nullptr) {
        if (trc->isCallbackTracer()) {
            trc_ = trc->asCallbackTracer();
            MOZ_ASSERT(trc_->contextIndex_ == CallbackTracer::InvalidIndex);
            trc_->contextIndex_ = initial;
        }
    }
    ~AutoTracingIndex() {
        if (trc_) {
            MOZ_ASSERT(trc_->contextIndex_ != CallbackTracer::InvalidIndex);
            trc_->contextIndex_ = CallbackTracer::InvalidIndex;
        }
    }

    void operator++() {
        if (trc_) {
            MOZ_ASSERT(trc_->contextIndex_ != CallbackTracer::InvalidIndex);
            ++trc_->contextIndex_;
        }
    }
};

class MOZ_RAII AutoTracingDetails
{
    CallbackTracer* trc_;

  public:
    AutoTracingDetails(JSTracer* trc, CallbackTracer::ContextFunctor& func) : trc_(nullptr) {
        if (trc->isCallbackTracer()) {
            trc_ = trc->asCallbackTracer();
            MOZ_ASSERT(trc_->contextFunctor_ == nullptr);
            trc_->contextFunctor_ = &func;
        }
    }
    ~AutoTracingDetails() {
        if (trc_) {
            MOZ_ASSERT(trc_->contextFunctor_);
            trc_->contextFunctor_ = nullptr;
        }
    }
};

} // namespace JS

JS::CallbackTracer*
JSTracer::asCallbackTracer()
{
    MOZ_ASSERT(isCallbackTracer());
    return static_cast<JS::CallbackTracer*>(this);
}

namespace JS {

template <typename T>
extern JS_PUBLIC_API(void)
TraceEdge(JSTracer* trc, JS::Heap<T>* edgep, const char* name);

extern JS_PUBLIC_API(void)
TraceEdge(JSTracer* trc, JS::TenuredHeap<JSObject*>* edgep, const char* name);

template <typename T>
extern JS_PUBLIC_API(void)
UnsafeTraceRoot(JSTracer* trc, T* edgep, const char* name);

extern JS_PUBLIC_API(void)
TraceChildren(JSTracer* trc, GCCellPtr thing);

using ZoneSet = js::HashSet<Zone*, js::DefaultHasher<Zone*>, js::SystemAllocPolicy>;
using CompartmentSet = js::HashSet<JSCompartment*, js::DefaultHasher<JSCompartment*>,
                                   js::SystemAllocPolicy>;

/**
 * Trace every value within |compartments| that is wrapped by a
 * cross-compartment wrapper from a compartment that is not an element of
 * |compartments|.
 */
extern JS_PUBLIC_API(void)
TraceIncomingCCWs(JSTracer* trc, const JS::CompartmentSet& compartments);

} // namespace JS

extern JS_PUBLIC_API(void)
JS_GetTraceThingInfo(char* buf, size_t bufsize, JSTracer* trc,
                     void* thing, JS::TraceKind kind, bool includeDetails);

namespace js {

template <typename T>
extern JS_PUBLIC_API(void)
UnsafeTraceManuallyBarrieredEdge(JSTracer* trc, T* edgep, const char* name);

namespace gc {

template <typename T>
extern JS_PUBLIC_API(bool)
EdgeNeedsSweep(JS::Heap<T>* edgep);

template <typename T>
bool
IsAboutToBeFinalizedUnbarriered(T* thingp);

} // namespace gc
} // namespace js

#endif /* js_TracingAPI_h */

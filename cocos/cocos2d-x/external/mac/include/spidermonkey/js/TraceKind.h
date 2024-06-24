/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_TraceKind_h
#define js_TraceKind_h

#include "mozilla/UniquePtr.h"

#include "js/TypeDecls.h"

namespace js {
class BaseShape;
class LazyScript;
class ObjectGroup;
class Shape;
class Scope;
namespace jit {
class JitCode;
} // namespace jit
} // namespace js

namespace JS {

enum class TraceKind
{
    Object = 0x00,
    String = 0x01,
    Symbol = 0x02,
    Script = 0x03,

    Shape = 0x04,

    ObjectGroup = 0x05,

    Null = 0x06,

    BaseShape = 0x0F,
    JitCode = 0x1F,
    LazyScript = 0x2F,
    Scope = 0x3F
};
const static uintptr_t OutOfLineTraceKindMask = 0x07;
static_assert(uintptr_t(JS::TraceKind::BaseShape) & OutOfLineTraceKindMask, "mask bits are set");
static_assert(uintptr_t(JS::TraceKind::JitCode) & OutOfLineTraceKindMask, "mask bits are set");
static_assert(uintptr_t(JS::TraceKind::LazyScript) & OutOfLineTraceKindMask, "mask bits are set");
static_assert(uintptr_t(JS::TraceKind::Scope) & OutOfLineTraceKindMask, "mask bits are set");

template <typename T>
struct MapTypeToTraceKind {
    static const JS::TraceKind kind = T::TraceKind;
};

#define JS_FOR_EACH_TRACEKIND(D) \
 /* PrettyName       TypeName           AddToCCKind */ \
    D(BaseShape,     js::BaseShape,     true) \
    D(JitCode,       js::jit::JitCode,  true) \
    D(LazyScript,    js::LazyScript,    true) \
    D(Scope,         js::Scope,         true) \
    D(Object,        JSObject,          true) \
    D(ObjectGroup,   js::ObjectGroup,   true) \
    D(Script,        JSScript,          true) \
    D(Shape,         js::Shape,         true) \
    D(String,        JSString,          false) \
    D(Symbol,        JS::Symbol,        false)

#define JS_EXPAND_DEF(name, type, _) \
    template <> struct MapTypeToTraceKind<type> { \
        static const JS::TraceKind kind = JS::TraceKind::name; \
    };
JS_FOR_EACH_TRACEKIND(JS_EXPAND_DEF);
#undef JS_EXPAND_DEF

enum class RootKind : int8_t
{
#define EXPAND_ROOT_KIND(name, _0, _1) \
    name,
JS_FOR_EACH_TRACEKIND(EXPAND_ROOT_KIND)
#undef EXPAND_ROOT_KIND

    Id,
    Value,

    Traceable,

    Limit
};

template <TraceKind traceKind> struct MapTraceKindToRootKind {};
#define JS_EXPAND_DEF(name, _0, _1) \
    template <> struct MapTraceKindToRootKind<JS::TraceKind::name> { \
        static const JS::RootKind kind = JS::RootKind::name; \
    };
JS_FOR_EACH_TRACEKIND(JS_EXPAND_DEF)
#undef JS_EXPAND_DEF

template <typename T>
struct MapTypeToRootKind {
    static const JS::RootKind kind = JS::RootKind::Traceable;
};
template <typename T>
struct MapTypeToRootKind<T*> {
    static const JS::RootKind kind =
        JS::MapTraceKindToRootKind<JS::MapTypeToTraceKind<T>::kind>::kind;
};
template <typename T>
struct MapTypeToRootKind<mozilla::UniquePtr<T>> {
    static const JS::RootKind kind = JS::MapTypeToRootKind<T>::kind;
};
template <> struct MapTypeToRootKind<JS::Value> {
    static const JS::RootKind kind = JS::RootKind::Value;
};
template <> struct MapTypeToRootKind<jsid> {
    static const JS::RootKind kind = JS::RootKind::Id;
};
template <> struct MapTypeToRootKind<JSFunction*> : public MapTypeToRootKind<JSObject*> {};


#if defined(_MSC_VER) && !defined(__clang__)
# define JS_DEPENDENT_TEMPLATE_HINT
#else
# define JS_DEPENDENT_TEMPLATE_HINT template
#endif
template <typename F, typename... Args>
auto
DispatchTraceKindTyped(F f, JS::TraceKind traceKind, Args&&... args)
  -> decltype(f. JS_DEPENDENT_TEMPLATE_HINT operator()<JSObject>(mozilla::Forward<Args>(args)...))
{
    switch (traceKind) {
#define JS_EXPAND_DEF(name, type, _) \
      case JS::TraceKind::name: \
        return f. JS_DEPENDENT_TEMPLATE_HINT operator()<type>(mozilla::Forward<Args>(args)...);
      JS_FOR_EACH_TRACEKIND(JS_EXPAND_DEF);
#undef JS_EXPAND_DEF
      default:
          MOZ_CRASH("Invalid trace kind in DispatchTraceKindTyped.");
    }
}
#undef JS_DEPENDENT_TEMPLATE_HINT

template <typename F, typename... Args>
auto
DispatchTraceKindTyped(F f, void* thing, JS::TraceKind traceKind, Args&&... args)
  -> decltype(f(static_cast<JSObject*>(nullptr), mozilla::Forward<Args>(args)...))
{
    switch (traceKind) {
#define JS_EXPAND_DEF(name, type, _) \
      case JS::TraceKind::name: \
          return f(static_cast<type*>(thing), mozilla::Forward<Args>(args)...);
      JS_FOR_EACH_TRACEKIND(JS_EXPAND_DEF);
#undef JS_EXPAND_DEF
      default:
          MOZ_CRASH("Invalid trace kind in DispatchTraceKindTyped.");
    }
}

} // namespace JS

#endif // js_TraceKind_h

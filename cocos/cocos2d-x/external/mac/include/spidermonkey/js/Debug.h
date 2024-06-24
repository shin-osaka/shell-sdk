/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef js_Debug_h
#define js_Debug_h

#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/MemoryReporting.h"

#include "jsapi.h"
#include "jspubtd.h"

#include "js/GCAPI.h"
#include "js/RootingAPI.h"
#include "js/TypeDecls.h"

namespace js {
class Debugger;
} // namespace js

namespace JS {
namespace dbg {


class BuilderOrigin;

class Builder {
    PersistentRootedObject debuggerObject;

    js::Debugger* debugger;

#if DEBUG
    void assertBuilt(JSObject* obj);
#else
    void assertBuilt(JSObject* obj) { }
#endif

  protected:
    template<typename T>
    class BuiltThing {
        friend class BuilderOrigin;

      protected:
        Builder& owner;

        PersistentRooted<T> value;

        BuiltThing(JSContext* cx, Builder& owner_, T value_ = GCPolicy<T>::initial())
          : owner(owner_), value(cx, value_)
        {
            owner.assertBuilt(value_);
        }

        js::Debugger* debugger() const { return owner.debugger; }
        JSObject* debuggerObject() const { return owner.debuggerObject; }

      public:
        BuiltThing(const BuiltThing& rhs) : owner(rhs.owner), value(rhs.value) { }
        BuiltThing& operator=(const BuiltThing& rhs) {
            MOZ_ASSERT(&owner == &rhs.owner);
            owner.assertBuilt(rhs.value);
            value = rhs.value;
            return *this;
        }

        explicit operator bool() const {
            return value;
        }

      private:
        BuiltThing() = delete;
    };

  public:
    class Object: private BuiltThing<JSObject*> {
        friend class Builder;           // for construction
        friend class BuilderOrigin;     // for unwrapping

        typedef BuiltThing<JSObject*> Base;

        Object(JSContext* cx, Builder& owner_, HandleObject obj) : Base(cx, owner_, obj.get()) { }

        bool definePropertyToTrusted(JSContext* cx, const char* name,
                                     JS::MutableHandleValue value);

      public:
        Object(JSContext* cx, Builder& owner_) : Base(cx, owner_, nullptr) { }
        Object(const Object& rhs) : Base(rhs) { }


        bool defineProperty(JSContext* cx, const char* name, JS::HandleValue value);
        bool defineProperty(JSContext* cx, const char* name, JS::HandleObject value);
        bool defineProperty(JSContext* cx, const char* name, Object& value);

        using Base::operator bool;
    };

    Object newObject(JSContext* cx);

  protected:
    Builder(JSContext* cx, js::Debugger* debugger);
};

class BuilderOrigin : public Builder {
    template<typename T>
    T unwrapAny(const BuiltThing<T>& thing) {
        MOZ_ASSERT(&thing.owner == this);
        return thing.value.get();
    }

  public:
    BuilderOrigin(JSContext* cx, js::Debugger* debugger_)
      : Builder(cx, debugger_)
    { }

    JSObject* unwrap(Object& object) { return unwrapAny(object); }
};




JS_PUBLIC_API(void)
SetDebuggerMallocSizeOf(JSContext* cx, mozilla::MallocSizeOf mallocSizeOf);

JS_PUBLIC_API(mozilla::MallocSizeOf)
GetDebuggerMallocSizeOf(JSContext* cx);





JS_PUBLIC_API(bool)
FireOnGarbageCollectionHook(JSContext* cx, GarbageCollectionEvent::Ptr&& data);




JS_PUBLIC_API(void)
onNewPromise(JSContext* cx, HandleObject promise);

JS_PUBLIC_API(void)
onPromiseSettled(JSContext* cx, HandleObject promise);



JS_PUBLIC_API(bool)
IsDebugger(JSObject& obj);

JS_PUBLIC_API(bool)
GetDebuggeeGlobals(JSContext* cx, JSObject& dbgObj, AutoObjectVector& vector);



class MOZ_STACK_CLASS AutoEntryMonitor {
    JSRuntime* runtime_;
    AutoEntryMonitor* savedMonitor_;

  public:
    explicit AutoEntryMonitor(JSContext* cx);
    ~AutoEntryMonitor();


    virtual void Entry(JSContext* cx, JSFunction* function,
                       HandleValue asyncStack,
                       const char* asyncCause) = 0;

    virtual void Entry(JSContext* cx, JSScript* script,
                       HandleValue asyncStack,
                       const char* asyncCause) = 0;

    virtual void Exit(JSContext* cx) { }
};



} // namespace dbg
} // namespace JS


#endif /* js_Debug_h */

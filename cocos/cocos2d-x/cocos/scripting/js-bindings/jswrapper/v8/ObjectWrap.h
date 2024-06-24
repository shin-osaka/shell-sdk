
#ifndef SRC_NODE_OBJECT_WRAP_H_
#define SRC_NODE_OBJECT_WRAP_H_

#include "../config.hpp"

#if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8

#include "Base.h"

namespace se {

    class ObjectWrap {
    public:
        ObjectWrap();
        ~ObjectWrap();

        bool init(v8::Local<v8::Object> handle);
        void setFinalizeCallback(V8FinalizeFunc finalizeCb);

        v8::Local<v8::Object> handle();
        v8::Local<v8::Object> handle(v8::Isolate *isolate);
        v8::Persistent<v8::Object> &persistent();

        void wrap(void *nativeObj);
        static void* unwrap(v8::Local<v8::Object> handle);

        /* Ref() marks the object as being attached to an event loop.
         * Refed objects will not be garbage collected, even if
         * all references are lost.
         */
        void ref();

        /* Unref() marks an object as detached from the event loop.  This is its
         * default state.  When an object with a "weak" reference changes from
         * attached to detached state it will be freed. Be careful not to access
         * the object after making this call as it might be gone!
         * (A "weak reference" means an object that only has a
         * persistent handle.)
         *
         * DO NOT CALL THIS FROM DESTRUCTOR
         */
        void unref();

    private:
        static void weakCallback(const v8::WeakCallbackInfo<ObjectWrap> &data);
        void makeWeak();

        int refs_;  // ro
        v8::Persistent<v8::Object> handle_;
        void *_nativeObj;
        V8FinalizeFunc _finalizeCb;
    };

}  // namespace se

#endif // #if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8

#endif  // SRC_NODE_OBJECT_WRAP_H_

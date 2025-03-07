/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "Class.hpp"

#if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8

#include "Object.hpp"
#include "Utils.hpp"
#include "ScriptEngine.hpp"

namespace se {

    namespace {
        v8::Isolate* __isolate = nullptr;
        std::vector<Class*> __allClasses;
    }

    Class::Class()
    : _parent(nullptr)
    , _parentProto(nullptr)
    , _proto(nullptr)
    , _ctor(nullptr)
    , _finalizeFunc(nullptr)
    , _createProto(true)
    {
        __allClasses.push_back(this);
    }

    Class::~Class()
    {
    }

    /* static */
    Class* Class::create(const std::string& clsName, se::Object* parent, Object* parentProto, v8::FunctionCallback ctor)
    {
        Class* cls = new Class();
        if (cls != nullptr && !cls->init(clsName, parent, parentProto, ctor))
        {
            delete cls;
            cls = nullptr;
        }
        return cls;
    }

    bool Class::init(const std::string& clsName, Object* parent, Object* parentProto, v8::FunctionCallback ctor)
    {
        _name = clsName;

        _parent = parent;
        if (_parent != nullptr)
            _parent->incRef();

        _parentProto = parentProto;
        if (_parentProto != nullptr)
            _parentProto->incRef();

        _ctor = ctor;

        _ctorTemplate.Reset(__isolate, v8::FunctionTemplate::New(__isolate, _ctor));
        v8::MaybeLocal<v8::String> jsNameVal = v8::String::NewFromUtf8(__isolate, _name.c_str(), v8::NewStringType::kNormal);
        if (jsNameVal.IsEmpty())
            return false;

        _ctorTemplate.Get(__isolate)->SetClassName(jsNameVal.ToLocalChecked());
        _ctorTemplate.Get(__isolate)->InstanceTemplate()->SetInternalFieldCount(1);

        return true;
    }

    void Class::destroy()
    {
        SAFE_DEC_REF(_parent);
        SAFE_DEC_REF(_proto);
        SAFE_DEC_REF(_parentProto);
        _ctorTemplate.Reset();
    }

    void Class::cleanup()
    {
        for (auto cls : __allClasses)
        {
            cls->destroy();
        }

        se::ScriptEngine::getInstance()->addAfterCleanupHook([](){
            for (auto cls : __allClasses)
            {
                delete cls;
            }
            __allClasses.clear();
        });
    }

    void Class::setCreateProto(bool createProto)
    {
        _createProto = createProto;
    }

    bool Class::install()
    {

        if (_parentProto != nullptr)
        {
            _ctorTemplate.Get(__isolate)->Inherit(_parentProto->_getClass()->_ctorTemplate.Get(__isolate));
        }

        v8::Local<v8::Context> context = __isolate->GetCurrentContext();
        v8::MaybeLocal<v8::Function> ctor = _ctorTemplate.Get(__isolate)->GetFunction(context);
        if (ctor.IsEmpty())
            return false;

        v8::Local<v8::Function> ctorChecked = ctor.ToLocalChecked();
        v8::MaybeLocal<v8::String> name = v8::String::NewFromUtf8(__isolate, _name.c_str(), v8::NewStringType::kNormal);
        if (name.IsEmpty())
            return false;

        v8::Maybe<bool> result = _parent->_getJSObject()->Set(context, name.ToLocalChecked(), ctorChecked);
        if (result.IsNothing())
            return false;

        v8::MaybeLocal<v8::String> prototypeName = v8::String::NewFromUtf8(__isolate, "prototype", v8::NewStringType::kNormal);
        if (prototypeName.IsEmpty())
            return false;

        v8::MaybeLocal<v8::Value> prototypeObj = ctorChecked->Get(context, prototypeName.ToLocalChecked());
        if (prototypeObj.IsEmpty())
            return false;

        if (_createProto)
        {
            _proto = Object::_createJSObject(this, v8::Local<v8::Object>::Cast(prototypeObj.ToLocalChecked()));
            _proto->root();
        }
        return true;
    }

    bool Class::defineFunction(const char *name, v8::FunctionCallback func)
    {
        v8::MaybeLocal<v8::String> jsName =  v8::String::NewFromUtf8(__isolate, name, v8::NewStringType::kNormal);
        if (jsName.IsEmpty())
            return false;
        _ctorTemplate.Get(__isolate)->PrototypeTemplate()->Set(jsName.ToLocalChecked(), v8::FunctionTemplate::New(__isolate, func));
        return true;
    }

    bool Class::defineProperty(const char *name, v8::AccessorNameGetterCallback getter, v8::AccessorNameSetterCallback setter)
    {
        v8::MaybeLocal<v8::String> jsName =  v8::String::NewFromUtf8(__isolate, name, v8::NewStringType::kNormal);
        if (jsName.IsEmpty())
            return false;
        _ctorTemplate.Get(__isolate)->PrototypeTemplate()->SetAccessor(jsName.ToLocalChecked(), getter, setter);
        return true;
    }

    bool Class::defineStaticFunction(const char *name, v8::FunctionCallback func)
    {
        v8::MaybeLocal<v8::String> jsName =  v8::String::NewFromUtf8(__isolate, name, v8::NewStringType::kNormal);
        if (jsName.IsEmpty())
            return false;
        _ctorTemplate.Get(__isolate)->Set(jsName.ToLocalChecked(), v8::FunctionTemplate::New(__isolate, func));
        return true;
    }

    bool Class::defineStaticProperty(const char *name, v8::AccessorNameGetterCallback getter, v8::AccessorNameSetterCallback setter)
    {
        v8::MaybeLocal<v8::String> jsName =  v8::String::NewFromUtf8(__isolate, name, v8::NewStringType::kNormal);
        if (jsName.IsEmpty())
            return false;
        _ctorTemplate.Get(__isolate)->SetNativeDataProperty(jsName.ToLocalChecked(), getter, setter);
        return true;
    }

    bool Class::defineFinalizeFunction(V8FinalizeFunc finalizeFunc)
    {
        assert(finalizeFunc != nullptr);
        _finalizeFunc = finalizeFunc;
        return true;
    }


    v8::Local<v8::Object> Class::_createJSObjectWithClass(Class* cls)
    {
        v8::MaybeLocal<v8::Object> ret = cls->_ctorTemplate.Get(__isolate)->InstanceTemplate()->NewInstance(__isolate->GetCurrentContext());
        assert(!ret.IsEmpty());
        return ret.ToLocalChecked();
    }

    Object* Class::getProto() const
    {
        return _proto;
    }

    V8FinalizeFunc Class::_getFinalizeFunction() const
    {
        return _finalizeFunc;
    }

    /* static */
    void Class::setIsolate(v8::Isolate* isolate)
    {
        __isolate = isolate;
    }

} // namespace se {

#endif // #if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8

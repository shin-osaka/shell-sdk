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
#include "MappingUtils.hpp"

namespace se {

NativePtrToObjectMap::Map* NativePtrToObjectMap::__nativePtrToObjectMap = nullptr;

bool NativePtrToObjectMap::init()
{
    if (__nativePtrToObjectMap == nullptr)
        __nativePtrToObjectMap = new (std::nothrow) NativePtrToObjectMap::Map();

    return __nativePtrToObjectMap != nullptr;
}

void NativePtrToObjectMap::destroy()
{
    if (__nativePtrToObjectMap != nullptr)
    {
        delete __nativePtrToObjectMap;
        __nativePtrToObjectMap = nullptr;
    }
}

void NativePtrToObjectMap::emplace(void* nativeObj, Object* seObj)
{
    __nativePtrToObjectMap->emplace(nativeObj, seObj);
}

NativePtrToObjectMap::Map::iterator NativePtrToObjectMap::find(void* nativeObj)
{
    return __nativePtrToObjectMap->find(nativeObj);
}

NativePtrToObjectMap::Map::iterator NativePtrToObjectMap::erase(Map::iterator iter)
{
    return __nativePtrToObjectMap->erase(iter);
}

void NativePtrToObjectMap::erase(void* nativeObj)
{
    __nativePtrToObjectMap->erase(nativeObj);
}

void NativePtrToObjectMap::clear()
{
    __nativePtrToObjectMap->clear();
}

size_t NativePtrToObjectMap::size()
{
    return __nativePtrToObjectMap->size();
}

const NativePtrToObjectMap::Map& NativePtrToObjectMap::instance()
{
    return *__nativePtrToObjectMap;
}

NativePtrToObjectMap::Map::iterator NativePtrToObjectMap::begin()
{
    return __nativePtrToObjectMap->begin();
}

NativePtrToObjectMap::Map::iterator NativePtrToObjectMap::end()
{
    return __nativePtrToObjectMap->end();
}


NonRefNativePtrCreatedByCtorMap::Map* NonRefNativePtrCreatedByCtorMap::__nonRefNativeObjectCreatedByCtorMap = nullptr;

bool NonRefNativePtrCreatedByCtorMap::init()
{
    if (__nonRefNativeObjectCreatedByCtorMap == nullptr)
        __nonRefNativeObjectCreatedByCtorMap = new (std::nothrow) NonRefNativePtrCreatedByCtorMap::Map();

    return __nonRefNativeObjectCreatedByCtorMap != nullptr;
}

void NonRefNativePtrCreatedByCtorMap::destroy()
{
    if (__nonRefNativeObjectCreatedByCtorMap != nullptr)
    {
        delete __nonRefNativeObjectCreatedByCtorMap;
        __nonRefNativeObjectCreatedByCtorMap = nullptr;
    }
}

void NonRefNativePtrCreatedByCtorMap::emplace(void* nativeObj)
{
    __nonRefNativeObjectCreatedByCtorMap->emplace(nativeObj, true);
}

NonRefNativePtrCreatedByCtorMap::Map::iterator NonRefNativePtrCreatedByCtorMap::find(void* nativeObj)
{
    return __nonRefNativeObjectCreatedByCtorMap->find(nativeObj);
}

NonRefNativePtrCreatedByCtorMap::Map::iterator NonRefNativePtrCreatedByCtorMap::erase(Map::iterator iter)
{
    return __nonRefNativeObjectCreatedByCtorMap->erase(iter);
}

void NonRefNativePtrCreatedByCtorMap::erase(void* nativeObj)
{
    __nonRefNativeObjectCreatedByCtorMap->erase(nativeObj);
}

void NonRefNativePtrCreatedByCtorMap::clear()
{
    __nonRefNativeObjectCreatedByCtorMap->clear();
}

size_t NonRefNativePtrCreatedByCtorMap::size()
{
    return __nonRefNativeObjectCreatedByCtorMap->size();
}

const NonRefNativePtrCreatedByCtorMap::Map& NonRefNativePtrCreatedByCtorMap::instance()
{
    return *__nonRefNativeObjectCreatedByCtorMap;
}

NonRefNativePtrCreatedByCtorMap::Map::iterator NonRefNativePtrCreatedByCtorMap::begin()
{
    return __nonRefNativeObjectCreatedByCtorMap->begin();
}

NonRefNativePtrCreatedByCtorMap::Map::iterator NonRefNativePtrCreatedByCtorMap::end()
{
    return __nonRefNativeObjectCreatedByCtorMap->end();
}



} // namespace se {

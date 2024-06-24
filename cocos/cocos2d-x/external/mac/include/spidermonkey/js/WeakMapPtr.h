/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_WeakMapPtr_h
#define js_WeakMapPtr_h

#include "jspubtd.h"

#include "js/TypeDecls.h"

namespace JS {

template <typename K, typename V>
class JS_PUBLIC_API(WeakMapPtr)
{
  public:
    WeakMapPtr() : ptr(nullptr) {}
    bool init(JSContext* cx);
    bool initialized() { return ptr != nullptr; }
    void destroy();
    virtual ~WeakMapPtr() { MOZ_ASSERT(!initialized()); }
    void trace(JSTracer* tracer);

    V lookup(const K& key);
    bool put(JSContext* cx, const K& key, const V& value);

  private:
    void* ptr;

    WeakMapPtr(const WeakMapPtr& wmp) = delete;
    WeakMapPtr& operator=(const WeakMapPtr& wmp) = delete;
};

} /* namespace JS */

#endif  /* js_WeakMapPtr_h */

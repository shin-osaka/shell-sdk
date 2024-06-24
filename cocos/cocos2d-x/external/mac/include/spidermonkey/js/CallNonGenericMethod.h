/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_CallNonGenericMethod_h
#define js_CallNonGenericMethod_h

#include "jstypes.h"

#include "js/CallArgs.h"

namespace JS {

typedef bool (*IsAcceptableThis)(HandleValue v);

typedef bool (*NativeImpl)(JSContext* cx, const CallArgs& args);

namespace detail {

extern JS_PUBLIC_API(bool)
CallMethodIfWrapped(JSContext* cx, IsAcceptableThis test, NativeImpl impl, const CallArgs& args);

} // namespace detail

template<IsAcceptableThis Test, NativeImpl Impl>
MOZ_ALWAYS_INLINE bool
CallNonGenericMethod(JSContext* cx, const CallArgs& args)
{
    HandleValue thisv = args.thisv();
    if (Test(thisv))
        return Impl(cx, args);

    return detail::CallMethodIfWrapped(cx, Test, Impl, args);
}

MOZ_ALWAYS_INLINE bool
CallNonGenericMethod(JSContext* cx, IsAcceptableThis Test, NativeImpl Impl, const CallArgs& args)
{
    HandleValue thisv = args.thisv();
    if (Test(thisv))
        return Impl(cx, args);

    return detail::CallMethodIfWrapped(cx, Test, Impl, args);
}

} // namespace JS

#endif /* js_CallNonGenericMethod_h */

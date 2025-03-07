/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_UniquePtr_h
#define js_UniquePtr_h

#include "mozilla/UniquePtr.h"

#include "js/Utility.h"

namespace js {

template <typename T, typename D = JS::DeletePolicy<T>>
using UniquePtr = mozilla::UniquePtr<T, D>;

namespace detail {

template<typename T>
struct UniqueSelector
{
  typedef UniquePtr<T> SingleObject;
};

template<typename T>
struct UniqueSelector<T[]>
{
  typedef UniquePtr<T[]> UnknownBound;
};

template<typename T, decltype(sizeof(int)) N>
struct UniqueSelector<T[N]>
{
  typedef UniquePtr<T[N]> KnownBound;
};

} // namespace detail

template<typename T, typename... Args>
typename detail::UniqueSelector<T>::SingleObject
MakeUnique(Args&&... aArgs)
{
  return UniquePtr<T>(js_new<T>(mozilla::Forward<Args>(aArgs)...));
}

template<typename T>
typename detail::UniqueSelector<T>::UnknownBound
MakeUnique(decltype(sizeof(int)) aN) = delete;

template<typename T, typename... Args>
typename detail::UniqueSelector<T>::KnownBound
MakeUnique(Args&&... aArgs) = delete;

} // namespace js

#endif /* js_UniquePtr_h */

/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_NotNull_h
#define mozilla_NotNull_h


#include "mozilla/Assertions.h"

namespace mozilla {

template <typename T>
class NotNull
{
  template <typename U> friend NotNull<U> WrapNotNull(U aBasePtr);

  T mBasePtr;

  template <typename U>
  explicit NotNull(U aBasePtr) : mBasePtr(aBasePtr) {}

public:
  NotNull() = delete;

  template <typename U>
  MOZ_IMPLICIT NotNull(const NotNull<U>& aOther) : mBasePtr(aOther.get()) {}

  NotNull(const NotNull<T>&) = default;
  NotNull<T>& operator=(const NotNull<T>&) = default;
  NotNull(NotNull<T>&&) = default;
  NotNull<T>& operator=(NotNull<T>&&) = default;

  explicit operator bool() const = delete;

  const T& get() const { return mBasePtr; }

  operator const T&() const { return get(); }

  const T& operator->() const { return get(); }
  decltype(*mBasePtr) operator*() const { return *mBasePtr; }
};

template <typename T>
NotNull<T>
WrapNotNull(const T aBasePtr)
{
  NotNull<T> notNull(aBasePtr);
  MOZ_RELEASE_ASSERT(aBasePtr);
  return notNull;
}

template <typename T, typename U>
inline bool
operator==(const NotNull<T>& aLhs, const NotNull<U>& aRhs)
{
  return aLhs.get() == aRhs.get();
}
template <typename T, typename U>
inline bool
operator!=(const NotNull<T>& aLhs, const NotNull<U>& aRhs)
{
  return aLhs.get() != aRhs.get();
}

template <typename T, typename U>
inline bool
operator==(const NotNull<T>& aLhs, const U& aRhs)
{
  return aLhs.get() == aRhs;
}
template <typename T, typename U>
inline bool
operator!=(const NotNull<T>& aLhs, const U& aRhs)
{
  return aLhs.get() != aRhs;
}

template <typename T, typename U>
inline bool
operator==(const T& aLhs, const NotNull<U>& aRhs)
{
  return aLhs == aRhs.get();
}
template <typename T, typename U>
inline bool
operator!=(const T& aLhs, const NotNull<U>& aRhs)
{
  return aLhs != aRhs.get();
}

template <typename T>
bool
operator==(const NotNull<T>&, decltype(nullptr)) = delete;
template <typename T>
bool
operator!=(const NotNull<T>&, decltype(nullptr)) = delete;

template <typename T>
bool
operator==(decltype(nullptr), const NotNull<T>&) = delete;
template <typename T>
bool
operator!=(decltype(nullptr), const NotNull<T>&) = delete;

} // namespace mozilla

#endif /* mozilla_NotNull_h */

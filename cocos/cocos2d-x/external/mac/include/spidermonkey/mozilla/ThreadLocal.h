/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Cross-platform lightweight thread local data wrappers. */

#ifndef mozilla_ThreadLocal_h
#define mozilla_ThreadLocal_h

#if defined(XP_WIN)
extern "C" {
__declspec(dllimport) void* __stdcall TlsGetValue(unsigned long);
__declspec(dllimport) int __stdcall TlsSetValue(unsigned long, void*);
__declspec(dllimport) unsigned long __stdcall TlsAlloc();
}
#else
#  include <pthread.h>
#  include <signal.h>
#endif

#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/TypeTraits.h"

namespace mozilla {

#if defined(XP_WIN)
typedef unsigned long sig_safe_t;
#else
typedef sig_atomic_t sig_safe_t;
#endif

namespace detail {

#if defined(HAVE_THREAD_TLS_KEYWORD)
#define MOZ_HAS_THREAD_LOCAL
#endif

/*
 * Thread Local Storage helpers.
 *
 * Usage:
 *
 * Do not directly instantiate this class.  Instead, use the
 * MOZ_THREAD_LOCAL macro to declare or define instances.  The macro
 * takes a type name as its argument.
 *
 * Declare like this:
 * extern MOZ_THREAD_LOCAL(int) tlsInt;
 * Define like this:
 * MOZ_THREAD_LOCAL(int) tlsInt;
 * or:
 * static MOZ_THREAD_LOCAL(int) tlsInt;
 *
 * Only static-storage-duration (e.g. global variables, or static class members)
 * objects of this class should be instantiated. This class relies on
 * zero-initialization, which is implicit for static-storage-duration objects.
 * It doesn't have a custom default constructor, to avoid static initializers.
 *
 * API usage:
 *
 * // Create a TLS item.
 * //
 * // Note that init() should be invoked before the first use of set()
 * // or get().  It is ok to call it multiple times.  This must be
 * // called in a way that avoids possible races with other threads.
 * MOZ_THREAD_LOCAL(int) tlsKey;
 * if (!tlsKey.init()) {
 *   // deal with the error
 * }
 *
 * // Set the TLS value
 * tlsKey.set(123);
 *
 * // Get the TLS value
 * int value = tlsKey.get();
 */
template<typename T>
class ThreadLocal
{
#ifndef MOZ_HAS_THREAD_LOCAL
#if defined(XP_WIN)
  typedef unsigned long key_t;
#else
  typedef pthread_key_t key_t;
#endif

  template<typename S>
  struct Helper
  {
    typedef uintptr_t Type;
  };

  template<typename S>
  struct Helper<S *>
  {
    typedef S *Type;
  };
#endif

  bool initialized() const {
#ifdef MOZ_HAS_THREAD_LOCAL
    return true;
#else
    return mInited;
#endif
  }

public:
#ifndef MOZ_HAS_THREAD_LOCAL
  ThreadLocal()
    : mKey(0), mInited(false)
  {}
#endif

  MOZ_MUST_USE inline bool init();

  inline T get() const;

  inline void set(const T aValue);

private:
#ifdef MOZ_HAS_THREAD_LOCAL
  T mValue;
#else
  key_t mKey;
  bool mInited;
#endif
};

template<typename T>
inline bool
ThreadLocal<T>::init()
{
  static_assert(mozilla::IsPointer<T>::value || mozilla::IsIntegral<T>::value,
                "mozilla::ThreadLocal must be used with a pointer or "
                "integral type");
  static_assert(sizeof(T) <= sizeof(void*),
                "mozilla::ThreadLocal can't be used for types larger than "
                "a pointer");

#ifdef MOZ_HAS_THREAD_LOCAL
  return true;
#else
  if (!initialized()) {
#ifdef XP_WIN
    mKey = TlsAlloc();
    mInited = mKey != 0xFFFFFFFFUL; // TLS_OUT_OF_INDEXES
#else
    mInited = !pthread_key_create(&mKey, nullptr);
#endif
  }
  return mInited;
#endif
}

template<typename T>
inline T
ThreadLocal<T>::get() const
{
#ifdef MOZ_HAS_THREAD_LOCAL
  return mValue;
#else
  MOZ_ASSERT(initialized());
  void* h;
#ifdef XP_WIN
  h = TlsGetValue(mKey);
#else
  h = pthread_getspecific(mKey);
#endif
  return static_cast<T>(reinterpret_cast<typename Helper<T>::Type>(h));
#endif
}

template<typename T>
inline void
ThreadLocal<T>::set(const T aValue)
{
#ifdef MOZ_HAS_THREAD_LOCAL
  mValue = aValue;
#else
  MOZ_ASSERT(initialized());
  void* h = reinterpret_cast<void*>(static_cast<typename Helper<T>::Type>(aValue));
#ifdef XP_WIN
  bool succeeded = TlsSetValue(mKey, h);
#else
  bool succeeded = !pthread_setspecific(mKey, h);
#endif
  if (!succeeded) {
    MOZ_CRASH();
  }
#endif
}

#ifdef MOZ_HAS_THREAD_LOCAL
#define MOZ_THREAD_LOCAL(TYPE) __thread mozilla::detail::ThreadLocal<TYPE>
#else
#define MOZ_THREAD_LOCAL(TYPE) mozilla::detail::ThreadLocal<TYPE>
#endif

} // namespace detail
} // namespace mozilla

#endif /* mozilla_ThreadLocal_h */

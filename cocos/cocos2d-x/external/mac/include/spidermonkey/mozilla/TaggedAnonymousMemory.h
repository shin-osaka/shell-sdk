/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef mozilla_TaggedAnonymousMemory_h
#define mozilla_TaggedAnonymousMemory_h

#ifndef XP_WIN

#include <sys/types.h>
#include <sys/mman.h>

#include "mozilla/Types.h"

#ifdef ANDROID

#ifdef __cplusplus
extern "C" {
#endif

MFBT_API void
MozTagAnonymousMemory(const void* aPtr, size_t aLength, const char* aTag);

MFBT_API void*
MozTaggedAnonymousMmap(void* aAddr, size_t aLength, int aProt, int aFlags,
                         int aFd, off_t aOffset, const char* aTag);

MFBT_API int
MozTaggedMemoryIsSupported(void);

#ifdef __cplusplus
} // extern "C"
#endif

#else // ANDROID

static inline void
MozTagAnonymousMemory(const void* aPtr, size_t aLength, const char* aTag)
{
}

static inline void*
MozTaggedAnonymousMmap(void* aAddr, size_t aLength, int aProt, int aFlags,
                       int aFd, off_t aOffset, const char* aTag)
{
  return mmap(aAddr, aLength, aProt, aFlags, aFd, aOffset);
}

static inline int
MozTaggedMemoryIsSupported(void)
{
  return 0;
}

#endif // ANDROID

#endif // !XP_WIN

#endif // mozilla_TaggedAnonymousMemory_h

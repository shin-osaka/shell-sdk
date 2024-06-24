/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_BufferList_h
#define mozilla_BufferList_h

#include <algorithm>
#include "mozilla/AllocPolicy.h"
#include "mozilla/Move.h"
#include "mozilla/ScopeExit.h"
#include "mozilla/Types.h"
#include "mozilla/TypeTraits.h"
#include "mozilla/Vector.h"
#include <string.h>


namespace mozilla {

template<typename AllocPolicy>
class BufferList : private AllocPolicy
{
  struct Segment
  {
    char* mData;
    size_t mSize;
    size_t mCapacity;

    Segment(char* aData, size_t aSize, size_t aCapacity)
     : mData(aData),
       mSize(aSize),
       mCapacity(aCapacity)
    {
    }

    Segment(const Segment&) = delete;
    Segment& operator=(const Segment&) = delete;

    Segment(Segment&&) = default;
    Segment& operator=(Segment&&) = default;

    char* Start() const { return mData; }
    char* End() const { return mData + mSize; }
  };

  template<typename OtherAllocPolicy>
  friend class BufferList;

 public:
  static const size_t kSegmentAlignment = 8;

  BufferList(size_t aInitialSize,
             size_t aInitialCapacity,
             size_t aStandardCapacity,
             AllocPolicy aAP = AllocPolicy())
   : AllocPolicy(aAP),
     mOwning(true),
     mSegments(aAP),
     mSize(0),
     mStandardCapacity(aStandardCapacity)
  {
    MOZ_ASSERT(aInitialCapacity % kSegmentAlignment == 0);
    MOZ_ASSERT(aStandardCapacity % kSegmentAlignment == 0);

    if (aInitialCapacity) {
      AllocateSegment(aInitialSize, aInitialCapacity);
    }
  }

  BufferList(const BufferList& aOther) = delete;

  BufferList(BufferList&& aOther)
   : mOwning(aOther.mOwning),
     mSegments(Move(aOther.mSegments)),
     mSize(aOther.mSize),
     mStandardCapacity(aOther.mStandardCapacity)
  {
    aOther.mSegments.clear();
    aOther.mSize = 0;
  }

  BufferList& operator=(const BufferList& aOther) = delete;

  BufferList& operator=(BufferList&& aOther)
  {
    Clear();

    mOwning = aOther.mOwning;
    mSegments = Move(aOther.mSegments);
    mSize = aOther.mSize;
    aOther.mSegments.clear();
    aOther.mSize = 0;
    return *this;
  }

  ~BufferList() { Clear(); }

  size_t Size() const { return mSize; }

  void Clear()
  {
    if (mOwning) {
      for (Segment& segment : mSegments) {
        this->free_(segment.mData);
      }
    }
    mSegments.clear();

    mSize = 0;
  }

  class IterImpl
  {
    uintptr_t mSegment;
    char* mData;
    char* mDataEnd;

    friend class BufferList;

  public:
    explicit IterImpl(const BufferList& aBuffers)
     : mSegment(0),
       mData(nullptr),
       mDataEnd(nullptr)
    {
      if (!aBuffers.mSegments.empty()) {
        mData = aBuffers.mSegments[0].Start();
        mDataEnd = aBuffers.mSegments[0].End();
      }
    }

    char* Data() const
    {
      MOZ_RELEASE_ASSERT(!Done());
      return mData;
    }

    bool HasRoomFor(size_t aBytes) const
    {
      MOZ_RELEASE_ASSERT(mData <= mDataEnd);
      return size_t(mDataEnd - mData) >= aBytes;
    }

    size_t RemainingInSegment() const
    {
      MOZ_RELEASE_ASSERT(mData <= mDataEnd);
      return mDataEnd - mData;
    }

    void Advance(const BufferList& aBuffers, size_t aBytes)
    {
      const Segment& segment = aBuffers.mSegments[mSegment];
      MOZ_RELEASE_ASSERT(segment.Start() <= mData);
      MOZ_RELEASE_ASSERT(mData <= mDataEnd);
      MOZ_RELEASE_ASSERT(mDataEnd == segment.End());

      MOZ_RELEASE_ASSERT(HasRoomFor(aBytes));
      mData += aBytes;

      if (mData == mDataEnd && mSegment + 1 < aBuffers.mSegments.length()) {
        mSegment++;
        const Segment& nextSegment = aBuffers.mSegments[mSegment];
        mData = nextSegment.Start();
        mDataEnd = nextSegment.End();
        MOZ_RELEASE_ASSERT(mData < mDataEnd);
      }
    }

    bool AdvanceAcrossSegments(const BufferList& aBuffers, size_t aBytes)
    {
      size_t bytes = aBytes;
      while (bytes) {
        size_t toAdvance = std::min(bytes, RemainingInSegment());
        if (!toAdvance) {
          return false;
        }
        Advance(aBuffers, toAdvance);
        bytes -= toAdvance;
      }
      return true;
    }

    bool Done() const
    {
      return mData == mDataEnd;
    }

   private:

    size_t BytesUntil(const BufferList& aBuffers, const IterImpl& aTarget) const {
      size_t offset = 0;

      MOZ_ASSERT(aTarget.IsIn(aBuffers));

      char* data = mData;
      for (uintptr_t segment = mSegment; segment < aTarget.mSegment; segment++) {
        offset += aBuffers.mSegments[segment].End() - data;
        data = aBuffers.mSegments[segment].mData;
      }

      MOZ_RELEASE_ASSERT(IsIn(aBuffers));
      MOZ_RELEASE_ASSERT(aTarget.mData >= data);

      offset += aTarget.mData - data;
      return offset;
    }

    bool IsIn(const BufferList& aBuffers) const {
      return mSegment < aBuffers.mSegments.length() &&
             mData >= aBuffers.mSegments[mSegment].mData &&
             mData < aBuffers.mSegments[mSegment].End();
    }
  };

  char* Start() { return mSegments[0].mData; }
  const char* Start() const { return mSegments[0].mData; }

  IterImpl Iter() const { return IterImpl(*this); }

  inline bool WriteBytes(const char* aData, size_t aSize);

  inline bool ReadBytes(IterImpl& aIter, char* aData, size_t aSize) const;

  template<typename BorrowingAllocPolicy>
  BufferList<BorrowingAllocPolicy> Borrow(IterImpl& aIter, size_t aSize, bool* aSuccess,
                                          BorrowingAllocPolicy aAP = BorrowingAllocPolicy()) const;

  template<typename OtherAllocPolicy>
  BufferList<OtherAllocPolicy> MoveFallible(bool* aSuccess, OtherAllocPolicy aAP = OtherAllocPolicy());

  BufferList Extract(IterImpl& aIter, size_t aSize, bool* aSuccess);

  size_t RangeLength(const IterImpl& start, const IterImpl& end) const {
    MOZ_ASSERT(start.IsIn(*this) && end.IsIn(*this));
    return start.BytesUntil(*this, end);
  }

private:
  explicit BufferList(AllocPolicy aAP)
   : AllocPolicy(aAP),
     mOwning(false),
     mSize(0),
     mStandardCapacity(0)
  {
  }

  void* AllocateSegment(size_t aSize, size_t aCapacity)
  {
    MOZ_RELEASE_ASSERT(mOwning);

    char* data = this->template pod_malloc<char>(aCapacity);
    if (!data) {
      return nullptr;
    }
    if (!mSegments.append(Segment(data, aSize, aCapacity))) {
      this->free_(data);
      return nullptr;
    }
    mSize += aSize;
    return data;
  }

  bool mOwning;
  Vector<Segment, 1, AllocPolicy> mSegments;
  size_t mSize;
  size_t mStandardCapacity;
};

template<typename AllocPolicy>
bool
BufferList<AllocPolicy>::WriteBytes(const char* aData, size_t aSize)
{
  MOZ_RELEASE_ASSERT(mOwning);
  MOZ_RELEASE_ASSERT(mStandardCapacity);

  size_t copied = 0;
  size_t remaining = aSize;

  if (!mSegments.empty()) {
    Segment& lastSegment = mSegments.back();

    size_t toCopy = std::min(aSize, lastSegment.mCapacity - lastSegment.mSize);
    memcpy(lastSegment.mData + lastSegment.mSize, aData, toCopy);
    lastSegment.mSize += toCopy;
    mSize += toCopy;

    copied += toCopy;
    remaining -= toCopy;
  }

  while (remaining) {
    size_t toCopy = std::min(remaining, mStandardCapacity);

    void* data = AllocateSegment(toCopy, mStandardCapacity);
    if (!data) {
      return false;
    }
    memcpy(data, aData + copied, toCopy);

    copied += toCopy;
    remaining -= toCopy;
  }

  return true;
}

template<typename AllocPolicy>
bool
BufferList<AllocPolicy>::ReadBytes(IterImpl& aIter, char* aData, size_t aSize) const
{
  size_t copied = 0;
  size_t remaining = aSize;
  while (remaining) {
    size_t toCopy = std::min(aIter.RemainingInSegment(), remaining);
    if (!toCopy) {
      return false;
    }
    memcpy(aData + copied, aIter.Data(), toCopy);
    copied += toCopy;
    remaining -= toCopy;

    aIter.Advance(*this, toCopy);
  }

  return true;
}

template<typename AllocPolicy> template<typename BorrowingAllocPolicy>
BufferList<BorrowingAllocPolicy>
BufferList<AllocPolicy>::Borrow(IterImpl& aIter, size_t aSize, bool* aSuccess,
                                BorrowingAllocPolicy aAP) const
{
  BufferList<BorrowingAllocPolicy> result(aAP);

  size_t size = aSize;
  while (size) {
    size_t toAdvance = std::min(size, aIter.RemainingInSegment());

    if (!toAdvance || !result.mSegments.append(typename BufferList<BorrowingAllocPolicy>::Segment(aIter.mData, toAdvance, toAdvance))) {
      *aSuccess = false;
      return result;
    }
    aIter.Advance(*this, toAdvance);
    size -= toAdvance;
  }

  result.mSize = aSize;
  *aSuccess = true;
  return result;
}

template<typename AllocPolicy> template<typename OtherAllocPolicy>
BufferList<OtherAllocPolicy>
BufferList<AllocPolicy>::MoveFallible(bool* aSuccess, OtherAllocPolicy aAP)
{
  BufferList<OtherAllocPolicy> result(0, 0, mStandardCapacity, aAP);

  IterImpl iter = Iter();
  while (!iter.Done()) {
    size_t toAdvance = iter.RemainingInSegment();

    if (!toAdvance || !result.mSegments.append(typename BufferList<OtherAllocPolicy>::Segment(iter.mData, toAdvance, toAdvance))) {
      *aSuccess = false;
      result.mSegments.clear();
      return result;
    }
    iter.Advance(*this, toAdvance);
  }

  result.mSize = mSize;
  mSegments.clear();
  mSize = 0;
  *aSuccess = true;
  return result;
}

template<typename AllocPolicy>
BufferList<AllocPolicy>
BufferList<AllocPolicy>::Extract(IterImpl& aIter, size_t aSize, bool* aSuccess)
{
  MOZ_RELEASE_ASSERT(aSize);
  MOZ_RELEASE_ASSERT(mOwning);
  MOZ_ASSERT(aSize % kSegmentAlignment == 0);
  MOZ_ASSERT(intptr_t(aIter.mData) % kSegmentAlignment == 0);

  IterImpl iter = aIter;
  size_t size = aSize;
  size_t toCopy = std::min(size, aIter.RemainingInSegment());
  MOZ_ASSERT(toCopy % kSegmentAlignment == 0);

  BufferList result(0, toCopy, mStandardCapacity);
  BufferList error(0, 0, mStandardCapacity);

  if (!result.WriteBytes(aIter.mData, toCopy)) {
    *aSuccess = false;
    return error;
  }
  iter.Advance(*this, toCopy);
  size -= toCopy;

  auto resultGuard = MakeScopeExit([&] {
    *aSuccess = false;
    result.mSegments.erase(result.mSegments.begin()+1, result.mSegments.end());
  });

  size_t movedSize = 0;
  uintptr_t toRemoveStart = iter.mSegment;
  uintptr_t toRemoveEnd = iter.mSegment;
  while (!iter.Done() &&
         !iter.HasRoomFor(size)) {
    if (!result.mSegments.append(Segment(mSegments[iter.mSegment].mData,
                                         mSegments[iter.mSegment].mSize,
                                         mSegments[iter.mSegment].mCapacity))) {
      return error;
    }
    movedSize += iter.RemainingInSegment();
    size -= iter.RemainingInSegment();
    toRemoveEnd++;
    iter.Advance(*this, iter.RemainingInSegment());
  }

  if (size)  {
    if (!iter.HasRoomFor(size) ||
        !result.WriteBytes(iter.Data(), size)) {
      return error;
    }
    iter.Advance(*this, size);
  }

  mSegments.erase(mSegments.begin() + toRemoveStart, mSegments.begin() + toRemoveEnd);
  mSize -= movedSize;
  aIter.mSegment = iter.mSegment - (toRemoveEnd - toRemoveStart);
  aIter.mData = iter.mData;
  aIter.mDataEnd = iter.mDataEnd;
  MOZ_ASSERT(aIter.mDataEnd == mSegments[aIter.mSegment].End());
  result.mSize = aSize;

  resultGuard.release();
  *aSuccess = true;
  return result;
}

} // namespace mozilla

#endif /* mozilla_BufferList_h */

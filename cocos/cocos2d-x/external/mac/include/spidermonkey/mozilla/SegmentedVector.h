/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef mozilla_SegmentedVector_h
#define mozilla_SegmentedVector_h

#include "mozilla/Alignment.h"
#include "mozilla/AllocPolicy.h"
#include "mozilla/Array.h"
#include "mozilla/LinkedList.h"
#include "mozilla/MemoryReporting.h"
#include "mozilla/Move.h"
#include "mozilla/TypeTraits.h"

#include <new>  // for placement new

namespace mozilla {

template<typename T,
         size_t IdealSegmentSize = 4096,
         typename AllocPolicy = MallocAllocPolicy>
class SegmentedVector : private AllocPolicy
{
  template<size_t SegmentCapacity>
  struct SegmentImpl
    : public mozilla::LinkedListElement<SegmentImpl<SegmentCapacity>>
  {
    SegmentImpl() : mLength(0) {}

    ~SegmentImpl()
    {
      for (uint32_t i = 0; i < mLength; i++) {
        (*this)[i].~T();
      }
    }

    uint32_t Length() const { return mLength; }

    T* Elems() { return reinterpret_cast<T*>(&mStorage.mBuf); }

    T& operator[](size_t aIndex)
    {
      MOZ_ASSERT(aIndex < mLength);
      return Elems()[aIndex];
    }

    const T& operator[](size_t aIndex) const
    {
      MOZ_ASSERT(aIndex < mLength);
      return Elems()[aIndex];
    }

    template<typename U>
    void Append(U&& aU)
    {
      MOZ_ASSERT(mLength < SegmentCapacity);
      mLength++;
      T* elem = &(*this)[mLength - 1];
      new (elem) T(mozilla::Forward<U>(aU));
    }

    void PopLast()
    {
      MOZ_ASSERT(mLength > 0);
      (*this)[mLength - 1].~T();
      mLength--;
    }

    uint32_t mLength;

    union Storage
    {
      char mBuf[sizeof(T) * SegmentCapacity];
      mozilla::AlignedElem<MOZ_ALIGNOF(T)> mAlign;
    } mStorage;

    static_assert(MOZ_ALIGNOF(T) == MOZ_ALIGNOF(Storage),
                  "SegmentedVector provides incorrect alignment");
  };

  static const size_t kSingleElementSegmentSize = sizeof(SegmentImpl<1>);
  static const size_t kSegmentCapacity =
    kSingleElementSegmentSize <= IdealSegmentSize
    ? (IdealSegmentSize - kSingleElementSegmentSize) / sizeof(T) + 1
    : 1;

  typedef SegmentImpl<kSegmentCapacity> Segment;

public:
  explicit SegmentedVector(size_t aIdealSegmentSize = 0)
  {
    MOZ_ASSERT_IF(
      aIdealSegmentSize != 0,
      (sizeof(Segment) > aIdealSegmentSize && kSegmentCapacity == 1) ||
      aIdealSegmentSize - sizeof(Segment) < sizeof(T));
  }

  ~SegmentedVector() { Clear(); }

  bool IsEmpty() const { return !mSegments.getFirst(); }

  size_t Length() const
  {
    size_t n = 0;
    for (auto segment = mSegments.getFirst();
         segment;
         segment = segment->getNext()) {
      n += segment->Length();
    }
    return n;
  }

  template<typename U>
  MOZ_MUST_USE bool Append(U&& aU)
  {
    Segment* last = mSegments.getLast();
    if (!last || last->Length() == kSegmentCapacity) {
      last = this->template pod_malloc<Segment>(1);
      if (!last) {
        return false;
      }
      new (last) Segment();
      mSegments.insertBack(last);
    }
    last->Append(mozilla::Forward<U>(aU));
    return true;
  }

  template<typename U>
  void InfallibleAppend(U&& aU)
  {
    bool ok = Append(mozilla::Forward<U>(aU));
    MOZ_RELEASE_ASSERT(ok);
  }

  void Clear()
  {
    Segment* segment;
    while ((segment = mSegments.popFirst())) {
      segment->~Segment();
      this->free_(segment);
    }
  }

  T& GetLast()
  {
    MOZ_ASSERT(!IsEmpty());
    Segment* last = mSegments.getLast();
    return (*last)[last->Length() - 1];
  }

  const T& GetLast() const
  {
    MOZ_ASSERT(!IsEmpty());
    Segment* last = mSegments.getLast();
    return (*last)[last->Length() - 1];
  }

  void PopLast()
  {
    MOZ_ASSERT(!IsEmpty());
    Segment* last = mSegments.getLast();
    last->PopLast();
    if (!last->Length()) {
      mSegments.popLast();
      last->~Segment();
      this->free_(last);
    }
  }

  void PopLastN(uint32_t aNumElements)
  {
    MOZ_ASSERT(aNumElements <= Length());

    Segment* last;

    do {
      last = mSegments.getLast();

      if (!last) {
        return;
      }

      uint32_t segmentLen = last->Length();
      if (segmentLen > aNumElements) {
        break;
      }

      mSegments.popLast();
      last->~Segment();
      this->free_(last);

      MOZ_ASSERT(aNumElements >= segmentLen);
      aNumElements -= segmentLen;
      if (aNumElements == 0) {
        return;
      }
    } while (true);

    MOZ_ASSERT(last);
    MOZ_ASSERT(last == mSegments.getLast());
    MOZ_ASSERT(aNumElements != 0);
    MOZ_ASSERT(aNumElements < last->Length());
    for (uint32_t i = 0; i < aNumElements; ++i) {
      last->PopLast();
    }
    MOZ_ASSERT(last->Length() != 0);
  }

  class IterImpl
  {
    friend class SegmentedVector;

    Segment* mSegment;
    size_t mIndex;

    explicit IterImpl(SegmentedVector* aVector)
      : mSegment(aVector->mSegments.getFirst())
      , mIndex(0)
    {}

  public:
    bool Done() const { return !mSegment; }

    T& Get()
    {
      MOZ_ASSERT(!Done());
      return (*mSegment)[mIndex];
    }

    const T& Get() const
    {
      MOZ_ASSERT(!Done());
      return (*mSegment)[mIndex];
    }

    void Next()
    {
      MOZ_ASSERT(!Done());
      mIndex++;
      if (mIndex == mSegment->Length()) {
        mSegment = mSegment->getNext();
        mIndex = 0;
      }
    }
  };

  IterImpl Iter() { return IterImpl(this); }

  size_t SizeOfExcludingThis(mozilla::MallocSizeOf aMallocSizeOf) const
  {
    return mSegments.sizeOfExcludingThis(aMallocSizeOf);
  }

  size_t SizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf) const
  {
    return aMallocSizeOf(this) + SizeOfExcludingThis(aMallocSizeOf);
  }

private:
  mozilla::LinkedList<Segment> mSegments;
};

} // namespace mozilla

#endif /* mozilla_SegmentedVector_h */

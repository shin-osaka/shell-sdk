/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_RefPtr_h
#define mozilla_RefPtr_h

#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"

/*****************************************************************************/


class nsCOMPtr_helper;

namespace mozilla {
template<class T> class OwningNonNull;
template<class T> class StaticRefPtr;

template<class U>
struct RefPtrTraits
{
  static void AddRef(U* aPtr) {
    aPtr->AddRef();
  }
  static void Release(U* aPtr) {
    aPtr->Release();
  }
};

} // namespace mozilla

template <class T>
class RefPtr
{
private:
  void
  assign_with_AddRef(T* aRawPtr)
  {
    if (aRawPtr) {
      ConstRemovingRefPtrTraits<T>::AddRef(aRawPtr);
    }
    assign_assuming_AddRef(aRawPtr);
  }

  void
  assign_assuming_AddRef(T* aNewPtr)
  {
    T* oldPtr = mRawPtr;
    mRawPtr = aNewPtr;
    if (oldPtr) {
      ConstRemovingRefPtrTraits<T>::Release(oldPtr);
    }
  }

private:
  T* MOZ_OWNING_REF mRawPtr;

public:
  typedef T element_type;

  ~RefPtr()
  {
    if (mRawPtr) {
      ConstRemovingRefPtrTraits<T>::Release(mRawPtr);
    }
  }


  RefPtr()
    : mRawPtr(nullptr)
  {
  }

  RefPtr(const RefPtr<T>& aSmartPtr)
    : mRawPtr(aSmartPtr.mRawPtr)
  {
    if (mRawPtr) {
      ConstRemovingRefPtrTraits<T>::AddRef(mRawPtr);
    }
  }

  RefPtr(RefPtr<T>&& aRefPtr)
    : mRawPtr(aRefPtr.mRawPtr)
  {
    aRefPtr.mRawPtr = nullptr;
  }


  MOZ_IMPLICIT RefPtr(T* aRawPtr)
    : mRawPtr(aRawPtr)
  {
    if (mRawPtr) {
      ConstRemovingRefPtrTraits<T>::AddRef(mRawPtr);
    }
  }

  MOZ_IMPLICIT RefPtr(decltype(nullptr))
    : mRawPtr(nullptr)
  {
  }

  template <typename I>
  MOZ_IMPLICIT RefPtr(already_AddRefed<I>& aSmartPtr)
    : mRawPtr(aSmartPtr.take())
  {
  }

  template <typename I>
  MOZ_IMPLICIT RefPtr(already_AddRefed<I>&& aSmartPtr)
    : mRawPtr(aSmartPtr.take())
  {
  }

  template <typename I>
  MOZ_IMPLICIT RefPtr(const RefPtr<I>& aSmartPtr)
    : mRawPtr(aSmartPtr.get())
  {
    if (mRawPtr) {
      ConstRemovingRefPtrTraits<T>::AddRef(mRawPtr);
    }
  }

  template <typename I>
  MOZ_IMPLICIT RefPtr(RefPtr<I>&& aSmartPtr)
    : mRawPtr(aSmartPtr.forget().take())
  {
  }

  MOZ_IMPLICIT RefPtr(const nsCOMPtr_helper& aHelper);

  template<class U>
  MOZ_IMPLICIT RefPtr(const mozilla::OwningNonNull<U>& aOther);

  template<class U>
  MOZ_IMPLICIT RefPtr(const mozilla::StaticRefPtr<U>& aOther);


  RefPtr<T>&
  operator=(decltype(nullptr))
  {
    assign_assuming_AddRef(nullptr);
    return *this;
  }

  RefPtr<T>&
  operator=(const RefPtr<T>& aRhs)
  {
    assign_with_AddRef(aRhs.mRawPtr);
    return *this;
  }

  template <typename I>
  RefPtr<T>&
  operator=(const RefPtr<I>& aRhs)
  {
    assign_with_AddRef(aRhs.get());
    return *this;
  }

  RefPtr<T>&
  operator=(T* aRhs)
  {
    assign_with_AddRef(aRhs);
    return *this;
  }

  template <typename I>
  RefPtr<T>&
  operator=(already_AddRefed<I>& aRhs)
  {
    assign_assuming_AddRef(aRhs.take());
    return *this;
  }

  template <typename I>
  RefPtr<T>&
  operator=(already_AddRefed<I> && aRhs)
  {
    assign_assuming_AddRef(aRhs.take());
    return *this;
  }

  RefPtr<T>& operator=(const nsCOMPtr_helper& aHelper);

  RefPtr<T>&
  operator=(RefPtr<T> && aRefPtr)
  {
    assign_assuming_AddRef(aRefPtr.mRawPtr);
    aRefPtr.mRawPtr = nullptr;
    return *this;
  }

  template<class U>
  RefPtr<T>&
  operator=(const mozilla::OwningNonNull<U>& aOther);

  template<class U>
  RefPtr<T>&
  operator=(const mozilla::StaticRefPtr<U>& aOther);


  void
  swap(RefPtr<T>& aRhs)
  {
    T* temp = aRhs.mRawPtr;
    aRhs.mRawPtr = mRawPtr;
    mRawPtr = temp;
  }

  void
  swap(T*& aRhs)
  {
    T* temp = aRhs;
    aRhs = mRawPtr;
    mRawPtr = temp;
  }

  already_AddRefed<T>
  forget()
  {
    T* temp = nullptr;
    swap(temp);
    return already_AddRefed<T>(temp);
  }

  template <typename I>
  void
  forget(I** aRhs)
  {
    MOZ_ASSERT(aRhs, "Null pointer passed to forget!");
    *aRhs = mRawPtr;
    mRawPtr = nullptr;
  }

  T*
  get() const
  /*
    Prefer the implicit conversion provided automatically by |operator T*() const|.
    Use |get()| to resolve ambiguity or to get a castable pointer.
  */
  {
    return const_cast<T*>(mRawPtr);
  }

  operator T*() const
#ifdef MOZ_HAVE_REF_QUALIFIERS
  &
#endif
  /*
    ...makes an |RefPtr| act like its underlying raw pointer type whenever it
    is used in a context where a raw pointer is expected.  It is this operator
    that makes an |RefPtr| substitutable for a raw pointer.

    Prefer the implicit use of this operator to calling |get()|, except where
    necessary to resolve ambiguity.
  */
  {
    return get();
  }

#ifdef MOZ_HAVE_REF_QUALIFIERS
  operator T*() const && = delete;

  explicit operator bool() const { return !!mRawPtr; }
  bool operator!() const { return !mRawPtr; }
#endif

  T*
  operator->() const MOZ_NO_ADDREF_RELEASE_ON_RETURN
  {
    MOZ_ASSERT(mRawPtr != nullptr,
               "You can't dereference a NULL RefPtr with operator->().");
    return get();
  }

  template <typename R, typename... Args>
  class Proxy
  {
    typedef R (T::*member_function)(Args...);
    T* mRawPtr;
    member_function mFunction;
  public:
    Proxy(T* aRawPtr, member_function aFunction)
      : mRawPtr(aRawPtr),
        mFunction(aFunction)
    {
    }
    template<typename... ActualArgs>
    R operator()(ActualArgs&&... aArgs)
    {
      return ((*mRawPtr).*mFunction)(mozilla::Forward<ActualArgs>(aArgs)...);
    }
  };

  template <typename R, typename... Args>
  Proxy<R, Args...> operator->*(R (T::*aFptr)(Args...)) const
  {
    MOZ_ASSERT(mRawPtr != nullptr,
               "You can't dereference a NULL RefPtr with operator->*().");
    return Proxy<R, Args...>(get(), aFptr);
  }

  RefPtr<T>*
  get_address()
  {
    return this;
  }

  const RefPtr<T>*
  get_address() const
  {
    return this;
  }

public:
  T&
  operator*() const
  {
    MOZ_ASSERT(mRawPtr != nullptr,
               "You can't dereference a NULL RefPtr with operator*().");
    return *get();
  }

  T**
  StartAssignment()
  {
    assign_assuming_AddRef(nullptr);
    return reinterpret_cast<T**>(&mRawPtr);
  }
private:
  template<class U>
  struct ConstRemovingRefPtrTraits
  {
    static void AddRef(U* aPtr) {
      mozilla::RefPtrTraits<U>::AddRef(aPtr);
    }
    static void Release(U* aPtr) {
      mozilla::RefPtrTraits<U>::Release(aPtr);
    }
  };
  template<class U>
  struct ConstRemovingRefPtrTraits<const U>
  {
    static void AddRef(const U* aPtr) {
      mozilla::RefPtrTraits<U>::AddRef(const_cast<U*>(aPtr));
    }
    static void Release(const U* aPtr) {
      mozilla::RefPtrTraits<U>::Release(const_cast<U*>(aPtr));
    }
  };
};

class nsCycleCollectionTraversalCallback;
template <typename T>
void
CycleCollectionNoteChild(nsCycleCollectionTraversalCallback& aCallback,
                         T* aChild, const char* aName, uint32_t aFlags);

template <typename T>
inline void
ImplCycleCollectionUnlink(RefPtr<T>& aField)
{
  aField = nullptr;
}

template <typename T>
inline void
ImplCycleCollectionTraverse(nsCycleCollectionTraversalCallback& aCallback,
                            RefPtr<T>& aField,
                            const char* aName,
                            uint32_t aFlags = 0)
{
  CycleCollectionNoteChild(aCallback, aField.get(), aName, aFlags);
}

template <class T>
inline RefPtr<T>*
address_of(RefPtr<T>& aPtr)
{
  return aPtr.get_address();
}

template <class T>
inline const RefPtr<T>*
address_of(const RefPtr<T>& aPtr)
{
  return aPtr.get_address();
}

template <class T>
class RefPtrGetterAddRefs
/*
  ...

  This class is designed to be used for anonymous temporary objects in the
  argument list of calls that return COM interface pointers, e.g.,

    RefPtr<IFoo> fooP;
    ...->GetAddRefedPointer(getter_AddRefs(fooP))

  DO NOT USE THIS TYPE DIRECTLY IN YOUR CODE.  Use |getter_AddRefs()| instead.

  When initialized with a |RefPtr|, as in the example above, it returns
  a |void**|, a |T**|, or an |nsISupports**| as needed, that the
  outer call (|GetAddRefedPointer| in this case) can fill in.

  This type should be a nested class inside |RefPtr<T>|.
*/
{
public:
  explicit
  RefPtrGetterAddRefs(RefPtr<T>& aSmartPtr)
    : mTargetSmartPtr(aSmartPtr)
  {
  }

  operator void**()
  {
    return reinterpret_cast<void**>(mTargetSmartPtr.StartAssignment());
  }

  operator T**()
  {
    return mTargetSmartPtr.StartAssignment();
  }

  T*&
  operator*()
  {
    return *(mTargetSmartPtr.StartAssignment());
  }

private:
  RefPtr<T>& mTargetSmartPtr;
};

template <class T>
inline RefPtrGetterAddRefs<T>
getter_AddRefs(RefPtr<T>& aSmartPtr)
/*
  Used around a |RefPtr| when
  ...makes the class |RefPtrGetterAddRefs<T>| invisible.
*/
{
  return RefPtrGetterAddRefs<T>(aSmartPtr);
}



template <class T, class U>
inline bool
operator==(const RefPtr<T>& aLhs, const RefPtr<U>& aRhs)
{
  return static_cast<const T*>(aLhs.get()) == static_cast<const U*>(aRhs.get());
}


template <class T, class U>
inline bool
operator!=(const RefPtr<T>& aLhs, const RefPtr<U>& aRhs)
{
  return static_cast<const T*>(aLhs.get()) != static_cast<const U*>(aRhs.get());
}



template <class T, class U>
inline bool
operator==(const RefPtr<T>& aLhs, const U* aRhs)
{
  return static_cast<const T*>(aLhs.get()) == static_cast<const U*>(aRhs);
}

template <class T, class U>
inline bool
operator==(const U* aLhs, const RefPtr<T>& aRhs)
{
  return static_cast<const U*>(aLhs) == static_cast<const T*>(aRhs.get());
}

template <class T, class U>
inline bool
operator!=(const RefPtr<T>& aLhs, const U* aRhs)
{
  return static_cast<const T*>(aLhs.get()) != static_cast<const U*>(aRhs);
}

template <class T, class U>
inline bool
operator!=(const U* aLhs, const RefPtr<T>& aRhs)
{
  return static_cast<const U*>(aLhs) != static_cast<const T*>(aRhs.get());
}

template <class T, class U>
inline bool
operator==(const RefPtr<T>& aLhs, U* aRhs)
{
  return static_cast<const T*>(aLhs.get()) == const_cast<const U*>(aRhs);
}

template <class T, class U>
inline bool
operator==(U* aLhs, const RefPtr<T>& aRhs)
{
  return const_cast<const U*>(aLhs) == static_cast<const T*>(aRhs.get());
}

template <class T, class U>
inline bool
operator!=(const RefPtr<T>& aLhs, U* aRhs)
{
  return static_cast<const T*>(aLhs.get()) != const_cast<const U*>(aRhs);
}

template <class T, class U>
inline bool
operator!=(U* aLhs, const RefPtr<T>& aRhs)
{
  return const_cast<const U*>(aLhs) != static_cast<const T*>(aRhs.get());
}


template <class T>
inline bool
operator==(const RefPtr<T>& aLhs, decltype(nullptr))
{
  return aLhs.get() == nullptr;
}

template <class T>
inline bool
operator==(decltype(nullptr), const RefPtr<T>& aRhs)
{
  return nullptr == aRhs.get();
}

template <class T>
inline bool
operator!=(const RefPtr<T>& aLhs, decltype(nullptr))
{
  return aLhs.get() != nullptr;
}

template <class T>
inline bool
operator!=(decltype(nullptr), const RefPtr<T>& aRhs)
{
  return nullptr != aRhs.get();
}

/*****************************************************************************/

template <class T>
inline already_AddRefed<T>
do_AddRef(T* aObj)
{
  RefPtr<T> ref(aObj);
  return ref.forget();
}

template <class T>
inline already_AddRefed<T>
do_AddRef(const RefPtr<T>& aObj)
{
  RefPtr<T> ref(aObj);
  return ref.forget();
}

namespace mozilla {

/**
 * Helper function to be able to conveniently write things like:
 *
 *   already_AddRefed<T>
 *   f(...)
 *   {
 *     return MakeAndAddRef<T>(...);
 *   }
 */
template<typename T, typename... Args>
already_AddRefed<T>
MakeAndAddRef(Args&&... aArgs)
{
  RefPtr<T> p(new T(Forward<Args>(aArgs)...));
  return p.forget();
}

} // namespace mozilla

#endif /* mozilla_RefPtr_h */

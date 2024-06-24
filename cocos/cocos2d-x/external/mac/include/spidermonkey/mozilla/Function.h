/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* A type-erased callable wrapper. */

#ifndef mozilla_Function_h
#define mozilla_Function_h

#include "mozilla/Attributes.h"  // for MOZ_IMPLICIT
#include "mozilla/Move.h"
#include "mozilla/RefCounted.h"
#include "mozilla/RefPtr.h"


namespace mozilla {

namespace detail {

template<typename ReturnType, typename... Arguments>
class FunctionImplBase : public mozilla::RefCounted<FunctionImplBase<ReturnType, Arguments...>>
{
public:
  MOZ_DECLARE_REFCOUNTED_TYPENAME(FunctionImplBase)

  virtual ~FunctionImplBase() {}
  virtual ReturnType call(Arguments... aArguments) = 0;
};

template <typename Callable, typename ReturnType, typename... Arguments>
class FunctionImpl : public FunctionImplBase<ReturnType, Arguments...>
{
  public:
    explicit FunctionImpl(const Callable& aCallable)
      : mCallable(aCallable) {}

    ReturnType call(Arguments... aArguments) override
    {
      return mCallable(Forward<Arguments>(aArguments)...);
    }
  private:
    Callable mCallable;
};

template <typename Callable, typename ReturnType, typename... Arguments>
class MemberFunctionImplBase : public FunctionImplBase<ReturnType, Arguments...>
{
public:
  explicit MemberFunctionImplBase(const Callable& aCallable)
    : mCallable(aCallable) {}

  ReturnType call(Arguments... aArguments) override
  {
    return callInternal(Forward<Arguments>(aArguments)...);
  }
private:
  template<typename ThisType, typename... Args>
  ReturnType callInternal(ThisType* aThis, Args&&... aArguments)
  {
    return (aThis->*mCallable)(Forward<Args>(aArguments)...);
  }

  template<typename ThisType, typename... Args>
  ReturnType callInternal(ThisType&& aThis, Args&&... aArguments)
  {
    return (aThis.*mCallable)(Forward<Args>(aArguments)...);
  }
  Callable mCallable;
};

template <typename ThisType, typename... Args, typename ReturnType, typename... Arguments>
class FunctionImpl<ReturnType(ThisType::*)(Args...),
                   ReturnType, Arguments...>
  : public MemberFunctionImplBase<ReturnType(ThisType::*)(Args...),
                                  ReturnType, Arguments...>
{
public:
  explicit FunctionImpl(ReturnType(ThisType::*aMemberFunc)(Args...))
    : MemberFunctionImplBase<ReturnType(ThisType::*)(Args...),
                             ReturnType, Arguments...>(aMemberFunc)
  {}
};

template <typename ThisType, typename... Args, typename ReturnType, typename... Arguments>
class FunctionImpl<ReturnType(ThisType::*)(Args...) const,
                   ReturnType, Arguments...>
  : public MemberFunctionImplBase<ReturnType(ThisType::*)(Args...) const,
                                  ReturnType, Arguments...>
{
public:
  explicit FunctionImpl(ReturnType(ThisType::*aConstMemberFunc)(Args...) const)
    : MemberFunctionImplBase<ReturnType(ThisType::*)(Args...) const,
                             ReturnType, Arguments...>(aConstMemberFunc)
  {}
};

} // namespace detail

template<typename Signature>
class function;

template<typename ReturnType, typename... Arguments>
class function<ReturnType(Arguments...)>
{
public:
  function() {}

  template <typename Callable>
  MOZ_IMPLICIT function(const Callable& aCallable)
    : mImpl(new detail::FunctionImpl<Callable, ReturnType, Arguments...>(aCallable))
  {}
  MOZ_IMPLICIT function(const function& aFunction)
    : mImpl(aFunction.mImpl)
  {}
  MOZ_IMPLICIT function(decltype(nullptr))
  {}

  function(function&& aOther) : mImpl(Move(aOther.mImpl)) {}
  function& operator=(function&& aOther) {
    mImpl = Move(aOther.mImpl);
    return *this;
  }

  template <typename Callable>
  function& operator=(const Callable& aCallable)
  {
    mImpl = new detail::FunctionImpl<Callable, ReturnType, Arguments...>(aCallable);
    return *this;
  }
  function& operator=(const function& aFunction)
  {
    mImpl = aFunction.mImpl;
    return *this;
  }
  function& operator=(decltype(nullptr))
  {
    mImpl = nullptr;
    return *this;
  }

  template<typename... Args>
  ReturnType operator()(Args&&... aArguments) const
  {
    MOZ_ASSERT(mImpl);
    return mImpl->call(Forward<Args>(aArguments)...);
  }

  explicit operator bool() const
  {
    return bool(mImpl);
  }

private:
  RefPtr<detail::FunctionImplBase<ReturnType, Arguments...>> mImpl;
};

template<typename Signature>
bool
operator==(const function<Signature>& aX, decltype(nullptr))
{
  return !aX;
}

template<typename Signature>
bool
operator==(decltype(nullptr), const function<Signature>& aX)
{
  return !aX;
}

template<typename Signature>
bool
operator!=(const function<Signature>& aX, decltype(nullptr))
{
  return bool(aX);
}

template<typename Signature>
bool
operator!=(decltype(nullptr), const function<Signature>& aX)
{
  return bool(aX);
}

} // namespace mozilla

#endif /* mozilla_Function_h */

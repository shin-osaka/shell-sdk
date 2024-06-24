/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_TimeStamp_h
#define mozilla_TimeStamp_h

#include <stdint.h>
#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/FloatingPoint.h"
#include "mozilla/TypeTraits.h"
#include "mozilla/Types.h"

namespace IPC {
template<typename T> struct ParamTraits;
} // namespace IPC

#ifdef XP_WIN
#include "TimeStamp_windows.h"
#endif

namespace mozilla {

#ifndef XP_WIN
typedef uint64_t TimeStampValue;
#endif

class TimeStamp;

/**
 * Platform-specific implementation details of BaseTimeDuration.
 */
class BaseTimeDurationPlatformUtils
{
public:
  static MFBT_API double ToSeconds(int64_t aTicks);
  static MFBT_API double ToSecondsSigDigits(int64_t aTicks);
  static MFBT_API int64_t TicksFromMilliseconds(double aMilliseconds);
  static MFBT_API int64_t ResolutionInTicks();
};

/**
 * Instances of this class represent the length of an interval of time.
 * Negative durations are allowed, meaning the end is before the start.
 *
 * Internally the duration is stored as a int64_t in units of
 * PR_TicksPerSecond() when building with NSPR interval timers, or a
 * system-dependent unit when building with system clocks.  The
 * system-dependent unit must be constant, otherwise the semantics of
 * this class would be broken.
 *
 * The ValueCalculator template parameter determines how arithmetic
 * operations are performed on the integer count of ticks (mValue).
 */
template <typename ValueCalculator>
class BaseTimeDuration
{
public:
  constexpr BaseTimeDuration() : mValue(0) {}
  struct _SomethingVeryRandomHere;
  MOZ_IMPLICIT BaseTimeDuration(_SomethingVeryRandomHere* aZero) : mValue(0)
  {
    MOZ_ASSERT(!aZero, "Who's playing funny games here?");
  }

  template <typename E>
  explicit BaseTimeDuration(const BaseTimeDuration<E>& aOther)
    : mValue(aOther.mValue)
  { }

  template <typename E>
  BaseTimeDuration& operator=(const BaseTimeDuration<E>& aOther)
  {
    mValue = aOther.mValue;
    return *this;
  }

  double ToSeconds() const
  {
    if (mValue == INT64_MAX) {
      return PositiveInfinity<double>();
    }
    if (mValue == INT64_MIN) {
      return NegativeInfinity<double>();
    }
    return BaseTimeDurationPlatformUtils::ToSeconds(mValue);
  }
  double ToSecondsSigDigits() const
  {
    if (mValue == INT64_MAX) {
      return PositiveInfinity<double>();
    }
    if (mValue == INT64_MIN) {
      return NegativeInfinity<double>();
    }
    return BaseTimeDurationPlatformUtils::ToSecondsSigDigits(mValue);
  }
  double ToMilliseconds() const { return ToSeconds() * 1000.0; }
  double ToMicroseconds() const { return ToMilliseconds() * 1000.0; }

  static inline BaseTimeDuration FromSeconds(double aSeconds)
  {
    return FromMilliseconds(aSeconds * 1000.0);
  }
  static BaseTimeDuration FromMilliseconds(double aMilliseconds)
  {
    if (aMilliseconds == PositiveInfinity<double>()) {
      return Forever();
    }
    if (aMilliseconds == NegativeInfinity<double>()) {
      return FromTicks(INT64_MIN);
    }
    return FromTicks(
      BaseTimeDurationPlatformUtils::TicksFromMilliseconds(aMilliseconds));
  }
  static inline BaseTimeDuration FromMicroseconds(double aMicroseconds)
  {
    return FromMilliseconds(aMicroseconds / 1000.0);
  }

  static BaseTimeDuration Forever()
  {
    return FromTicks(INT64_MAX);
  }

  BaseTimeDuration operator+(const BaseTimeDuration& aOther) const
  {
    return FromTicks(ValueCalculator::Add(mValue, aOther.mValue));
  }
  BaseTimeDuration operator-(const BaseTimeDuration& aOther) const
  {
    return FromTicks(ValueCalculator::Subtract(mValue, aOther.mValue));
  }
  BaseTimeDuration& operator+=(const BaseTimeDuration& aOther)
  {
    mValue = ValueCalculator::Add(mValue, aOther.mValue);
    return *this;
  }
  BaseTimeDuration& operator-=(const BaseTimeDuration& aOther)
  {
    mValue = ValueCalculator::Subtract(mValue, aOther.mValue);
    return *this;
  }
  BaseTimeDuration operator-() const
  {
    int64_t ticks;
    if (mValue == INT64_MAX) {
      ticks = INT64_MIN;
    } else if (mValue == INT64_MIN) {
      ticks = INT64_MAX;
    } else {
      ticks = -mValue;
    }

    return FromTicks(ticks);
  }

private:
  BaseTimeDuration operator*(const double aMultiplier) const = delete;

  BaseTimeDuration operator/(const double aDivisor) const = delete;

public:
  BaseTimeDuration MultDouble(double aMultiplier) const
  {
    return FromTicks(ValueCalculator::Multiply(mValue, aMultiplier));
  }
  BaseTimeDuration operator*(const int32_t aMultiplier) const
  {
    return FromTicks(ValueCalculator::Multiply(mValue, aMultiplier));
  }
  BaseTimeDuration operator*(const uint32_t aMultiplier) const
  {
    return FromTicks(ValueCalculator::Multiply(mValue, aMultiplier));
  }
  BaseTimeDuration operator*(const int64_t aMultiplier) const
  {
    return FromTicks(ValueCalculator::Multiply(mValue, aMultiplier));
  }
  BaseTimeDuration operator*(const uint64_t aMultiplier) const
  {
    if (aMultiplier > INT64_MAX) {
      return Forever();
    }
    return FromTicks(ValueCalculator::Multiply(mValue, aMultiplier));
  }
  BaseTimeDuration operator/(const int64_t aDivisor) const
  {
    MOZ_ASSERT(aDivisor != 0, "Division by zero");
    return FromTicks(ValueCalculator::Divide(mValue, aDivisor));
  }
  double operator/(const BaseTimeDuration& aOther) const
  {
#ifndef MOZ_B2G
    MOZ_ASSERT(aOther.mValue != 0, "Division by zero");
#endif
    return ValueCalculator::DivideDouble(mValue, aOther.mValue);
  }
  BaseTimeDuration operator%(const BaseTimeDuration& aOther) const
  {
    MOZ_ASSERT(aOther.mValue != 0, "Division by zero");
    return FromTicks(ValueCalculator::Modulo(mValue, aOther.mValue));
  }

  template<typename E>
  bool operator<(const BaseTimeDuration<E>& aOther) const
  {
    return mValue < aOther.mValue;
  }
  template<typename E>
  bool operator<=(const BaseTimeDuration<E>& aOther) const
  {
    return mValue <= aOther.mValue;
  }
  template<typename E>
  bool operator>=(const BaseTimeDuration<E>& aOther) const
  {
    return mValue >= aOther.mValue;
  }
  template<typename E>
  bool operator>(const BaseTimeDuration<E>& aOther) const
  {
    return mValue > aOther.mValue;
  }
  template<typename E>
  bool operator==(const BaseTimeDuration<E>& aOther) const
  {
    return mValue == aOther.mValue;
  }
  template<typename E>
  bool operator!=(const BaseTimeDuration<E>& aOther) const
  {
    return mValue != aOther.mValue;
  }
  bool IsZero() const
  {
    return mValue == 0;
  }
  explicit operator bool() const
  {
    return mValue != 0;
  }

  static BaseTimeDuration Resolution() {
    return FromTicks(BaseTimeDurationPlatformUtils::ResolutionInTicks());
  }


private:
  friend class TimeStamp;
  friend struct IPC::ParamTraits<mozilla::BaseTimeDuration<ValueCalculator>>;
  template <typename>
  friend class BaseTimeDuration;

  static BaseTimeDuration FromTicks(int64_t aTicks)
  {
    BaseTimeDuration t;
    t.mValue = aTicks;
    return t;
  }

  static BaseTimeDuration FromTicks(double aTicks)
  {
    if (aTicks >= double(INT64_MAX)) {
      return FromTicks(INT64_MAX);
    }

    if (aTicks <= double(INT64_MIN)) {
      return FromTicks(INT64_MIN);
    }

    return FromTicks(int64_t(aTicks));
  }

  int64_t mValue;
};

/**
 * Perform arithmetic operations on the value of a BaseTimeDuration without
 * doing strict checks on the range of values.
 */
class TimeDurationValueCalculator
{
public:
  static int64_t Add(int64_t aA, int64_t aB) { return aA + aB; }
  static int64_t Subtract(int64_t aA, int64_t aB) { return aA - aB; }

  template <typename T>
  static int64_t Multiply(int64_t aA, T aB)
  {
    static_assert(IsIntegral<T>::value,
                  "Using integer multiplication routine with non-integer type."
                  " Further specialization required");
    return aA * static_cast<int64_t>(aB);
  }

  static int64_t Divide(int64_t aA, int64_t aB) { return aA / aB; }
  static double DivideDouble(int64_t aA, int64_t aB)
  {
    return static_cast<double>(aA) / aB;
  }
  static int64_t Modulo(int64_t aA, int64_t aB) { return aA % aB; }
};

template <>
inline int64_t
TimeDurationValueCalculator::Multiply<double>(int64_t aA, double aB)
{
  return static_cast<int64_t>(aA * aB);
}

/**
 * Specialization of BaseTimeDuration that uses TimeDurationValueCalculator for
 * arithmetic on the mValue member.
 *
 * Use this class for time durations that are *not* expected to hold values of
 * Forever (or the negative equivalent) or when such time duration are *not*
 * expected to be used in arithmetic operations.
 */
typedef BaseTimeDuration<TimeDurationValueCalculator> TimeDuration;

/**
 * Instances of this class represent moments in time, or a special
 * "null" moment. We do not use the non-monotonic system clock or
 * local time, since they can be reset, causing apparent backward
 * travel in time, which can confuse algorithms. Instead we measure
 * elapsed time according to the system.  This time can never go
 * backwards (i.e. it never wraps around, at least not in less than
 * five million years of system elapsed time). It might not advance
 * while the system is sleeping. If TimeStamp::SetNow() is not called
 * at all for hours or days, we might not notice the passage of some
 * of that time.
 *
 * We deliberately do not expose a way to convert TimeStamps to some
 * particular unit. All you can do is compute a difference between two
 * TimeStamps to get a TimeDuration. You can also add a TimeDuration
 * to a TimeStamp to get a new TimeStamp. You can't do something
 * meaningless like add two TimeStamps.
 *
 * Internally this is implemented as either a wrapper around
 *   - high-resolution, monotonic, system clocks if they exist on this
 *     platform
 *   - PRIntervalTime otherwise.  We detect wraparounds of
 *     PRIntervalTime and work around them.
 *
 * This class is similar to C++11's time_point, however it is
 * explicitly nullable and provides an IsNull() method. time_point
 * is initialized to the clock's epoch and provides a
 * time_since_epoch() method that functions similiarly. i.e.
 * t.IsNull() is equivalent to t.time_since_epoch() == decltype(t)::duration::zero();
 */
class TimeStamp
{
public:
  /**
   * Initialize to the "null" moment
   */
  constexpr TimeStamp() : mValue(0) {}

  /**
   * The system timestamps are the same as the TimeStamp
   * retrieved by mozilla::TimeStamp. Since we need this for
   * vsync timestamps, we enable the creation of mozilla::TimeStamps
   * on platforms that support vsync aligned refresh drivers / compositors
   * Verified true as of Jan 31, 2015: B2G and OS X
   * False on Windows 7
   * UNTESTED ON OTHER PLATFORMS
   */
#if defined(MOZ_WIDGET_GONK) || defined(XP_DARWIN)
  static TimeStamp FromSystemTime(int64_t aSystemTime)
  {
    static_assert(sizeof(aSystemTime) == sizeof(TimeStampValue),
                  "System timestamp should be same units as TimeStampValue");
    return TimeStamp(aSystemTime);
  }
#endif

  /**
   * Return true if this is the "null" moment
   */
  bool IsNull() const { return mValue == 0; }

  /**
   * Return true if this is not the "null" moment, may be used in tests, e.g.:
   * |if (timestamp) { ... }|
   */
  explicit operator bool() const
  {
    return mValue != 0;
  }

  /**
   * Return a timestamp reflecting the current elapsed system time. This
   * is monotonically increasing (i.e., does not decrease) over the
   * lifetime of this process' XPCOM session.
   *
   * Now() is trying to ensure the best possible precision on each platform,
   * at least one millisecond.
   *
   * NowLoRes() has been introduced to workaround performance problems of
   * QueryPerformanceCounter on the Windows platform.  NowLoRes() is giving
   * lower precision, usually 15.6 ms, but with very good performance benefit.
   * Use it for measurements of longer times, like >200ms timeouts.
   */
  static TimeStamp Now() { return Now(true); }
  static TimeStamp NowLoRes() { return Now(false); }

  /**
   * Return a timestamp representing the time when the current process was
   * created which will be comparable with other timestamps taken with this
   * class. If the actual process creation time is detected to be inconsistent
   * the @a aIsInconsistent parameter will be set to true, the returned
   * timestamp however will still be valid though inaccurate.
   *
   * @param aIsInconsistent Set to true if an inconsistency was detected in the
   * process creation time
   * @returns A timestamp representing the time when the process was created,
   * this timestamp is always valid even when errors are reported
   */
  static MFBT_API TimeStamp ProcessCreation(bool& aIsInconsistent);

  /**
   * Records a process restart. After this call ProcessCreation() will return
   * the time when the browser was restarted instead of the actual time when
   * the process was created.
   */
  static MFBT_API void RecordProcessRestart();

  /**
   * Compute the difference between two timestamps. Both must be non-null.
   */
  TimeDuration operator-(const TimeStamp& aOther) const
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    MOZ_ASSERT(!aOther.IsNull(), "Cannot compute with aOther null value");
    static_assert(-INT64_MAX > INT64_MIN, "int64_t sanity check");
    int64_t ticks = int64_t(mValue - aOther.mValue);
    if (mValue > aOther.mValue) {
      if (ticks < 0) {
        ticks = INT64_MAX;
      }
    } else {
      if (ticks > 0) {
        ticks = INT64_MIN;
      }
    }
    return TimeDuration::FromTicks(ticks);
  }

  TimeStamp operator+(const TimeDuration& aOther) const
  {
    TimeStamp result = *this;
    result += aOther;
    return result;
  }
  TimeStamp operator-(const TimeDuration& aOther) const
  {
    TimeStamp result = *this;
    result -= aOther;
    return result;
  }
  TimeStamp& operator+=(const TimeDuration& aOther)
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    TimeStampValue value = mValue + aOther.mValue;
    if (aOther.mValue < 0 && value > mValue) {
      value = 0;
    }
    mValue = value;
    return *this;
  }
  TimeStamp& operator-=(const TimeDuration& aOther)
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    TimeStampValue value = mValue - aOther.mValue;
    if (aOther.mValue > 0 && value > mValue) {
      value = 0;
    }
    mValue = value;
    return *this;
  }

  bool operator<(const TimeStamp& aOther) const
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    MOZ_ASSERT(!aOther.IsNull(), "Cannot compute with aOther null value");
    return mValue < aOther.mValue;
  }
  bool operator<=(const TimeStamp& aOther) const
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    MOZ_ASSERT(!aOther.IsNull(), "Cannot compute with aOther null value");
    return mValue <= aOther.mValue;
  }
  bool operator>=(const TimeStamp& aOther) const
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    MOZ_ASSERT(!aOther.IsNull(), "Cannot compute with aOther null value");
    return mValue >= aOther.mValue;
  }
  bool operator>(const TimeStamp& aOther) const
  {
    MOZ_ASSERT(!IsNull(), "Cannot compute with a null value");
    MOZ_ASSERT(!aOther.IsNull(), "Cannot compute with aOther null value");
    return mValue > aOther.mValue;
  }
  bool operator==(const TimeStamp& aOther) const
  {
    return IsNull()
           ? aOther.IsNull()
           : !aOther.IsNull() && mValue == aOther.mValue;
  }
  bool operator!=(const TimeStamp& aOther) const
  {
    return !(*this == aOther);
  }


  static MFBT_API void Startup();
  static MFBT_API void Shutdown();

private:
  friend struct IPC::ParamTraits<mozilla::TimeStamp>;
  friend void StartupTimelineRecordExternal(int, uint64_t);

  MOZ_IMPLICIT TimeStamp(TimeStampValue aValue) : mValue(aValue) {}

  static MFBT_API TimeStamp Now(bool aHighResolution);

  /**
   * Computes the uptime of the current process in microseconds. The result
   * is platform-dependent and needs to be checked against existing timestamps
   * for consistency.
   *
   * @returns The number of microseconds since the calling process was started
   *          or 0 if an error was encountered while computing the uptime
   */
  static MFBT_API uint64_t ComputeProcessUptime();

  /**
   * When built with PRIntervalTime, a value of 0 means this instance
   * is "null". Otherwise, the low 32 bits represent a PRIntervalTime,
   * and the high 32 bits represent a counter of the number of
   * rollovers of PRIntervalTime that we've seen. This counter starts
   * at 1 to avoid a real time colliding with the "null" value.
   *
   * PR_INTERVAL_MAX is set at 100,000 ticks per second. So the minimum
   * time to wrap around is about 2^64/100000 seconds, i.e. about
   * 5,849,424 years.
   *
   * When using a system clock, a value is system dependent.
   */
  TimeStampValue mValue;
};

} // namespace mozilla

#endif /* mozilla_TimeStamp_h */

/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* ECMAScript conversion operations. */

#ifndef js_Conversions_h
#define js_Conversions_h

#include "mozilla/Casting.h"
#include "mozilla/FloatingPoint.h"
#include "mozilla/TypeTraits.h"

#include <math.h>

#include "jspubtd.h"

#include "js/RootingAPI.h"
#include "js/Value.h"

struct JSContext;

namespace js {

/* DO NOT CALL THIS. Use JS::ToBoolean. */
extern JS_PUBLIC_API(bool)
ToBooleanSlow(JS::HandleValue v);

/* DO NOT CALL THIS.  Use JS::ToNumber. */
extern JS_PUBLIC_API(bool)
ToNumberSlow(JSContext* cx, JS::HandleValue v, double* dp);

/* DO NOT CALL THIS. Use JS::ToInt8. */
extern JS_PUBLIC_API(bool)
ToInt8Slow(JSContext *cx, JS::HandleValue v, int8_t *out);

/* DO NOT CALL THIS. Use JS::ToUint8. */
extern JS_PUBLIC_API(bool)
ToUint8Slow(JSContext *cx, JS::HandleValue v, uint8_t *out);

/* DO NOT CALL THIS. Use JS::ToInt16. */
extern JS_PUBLIC_API(bool)
ToInt16Slow(JSContext *cx, JS::HandleValue v, int16_t *out);

/* DO NOT CALL THIS. Use JS::ToInt32. */
extern JS_PUBLIC_API(bool)
ToInt32Slow(JSContext* cx, JS::HandleValue v, int32_t* out);

/* DO NOT CALL THIS. Use JS::ToUint32. */
extern JS_PUBLIC_API(bool)
ToUint32Slow(JSContext* cx, JS::HandleValue v, uint32_t* out);

/* DO NOT CALL THIS. Use JS::ToUint16. */
extern JS_PUBLIC_API(bool)
ToUint16Slow(JSContext* cx, JS::HandleValue v, uint16_t* out);

/* DO NOT CALL THIS. Use JS::ToInt64. */
extern JS_PUBLIC_API(bool)
ToInt64Slow(JSContext* cx, JS::HandleValue v, int64_t* out);

/* DO NOT CALL THIS. Use JS::ToUint64. */
extern JS_PUBLIC_API(bool)
ToUint64Slow(JSContext* cx, JS::HandleValue v, uint64_t* out);

/* DO NOT CALL THIS. Use JS::ToString. */
extern JS_PUBLIC_API(JSString*)
ToStringSlow(JSContext* cx, JS::HandleValue v);

/* DO NOT CALL THIS. Use JS::ToObject. */
extern JS_PUBLIC_API(JSObject*)
ToObjectSlow(JSContext* cx, JS::HandleValue v, bool reportScanStack);

} // namespace js

namespace JS {

namespace detail {

#ifdef JS_DEBUG
/**
 * Assert that we're not doing GC on cx, that we're in a request as
 * needed, and that the compartments for cx and v are correct.
 * Also check that GC would be safe at this point.
 */
extern JS_PUBLIC_API(void)
AssertArgumentsAreSane(JSContext* cx, HandleValue v);
#else
inline void AssertArgumentsAreSane(JSContext* cx, HandleValue v)
{}
#endif /* JS_DEBUG */

} // namespace detail

/**
 * ES6 draft 20141224, 7.1.1, second algorithm.
 *
 * Most users shouldn't call this -- use JS::ToBoolean, ToNumber, or ToString
 * instead.  This will typically only be called from custom convert hooks that
 * wish to fall back to the ES6 default conversion behavior shared by most
 * objects in JS, codified as OrdinaryToPrimitive.
 */
extern JS_PUBLIC_API(bool)
OrdinaryToPrimitive(JSContext* cx, HandleObject obj, JSType type, MutableHandleValue vp);

/* ES6 draft 20141224, 7.1.2. */
MOZ_ALWAYS_INLINE bool
ToBoolean(HandleValue v)
{
    if (v.isBoolean())
        return v.toBoolean();
    if (v.isInt32())
        return v.toInt32() != 0;
    if (v.isNullOrUndefined())
        return false;
    if (v.isDouble()) {
        double d = v.toDouble();
        return !mozilla::IsNaN(d) && d != 0;
    }
    if (v.isSymbol())
        return true;

    /* The slow path handles strings and objects. */
    return js::ToBooleanSlow(v);
}

/* ES6 draft 20141224, 7.1.3. */
MOZ_ALWAYS_INLINE bool
ToNumber(JSContext* cx, HandleValue v, double* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isNumber()) {
        *out = v.toNumber();
        return true;
    }
    return js::ToNumberSlow(cx, v, out);
}

/* ES6 draft 20141224, ToInteger (specialized for doubles). */
inline double
ToInteger(double d)
{
    if (d == 0)
        return d;

    if (!mozilla::IsFinite(d)) {
        if (mozilla::IsNaN(d))
            return 0;
        return d;
    }

    return d < 0 ? ceil(d) : floor(d);
}

/* ES6 draft 20141224, 7.1.5. */
MOZ_ALWAYS_INLINE bool
ToInt32(JSContext* cx, JS::HandleValue v, int32_t* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = v.toInt32();
        return true;
    }
    return js::ToInt32Slow(cx, v, out);
}

/* ES6 draft 20141224, 7.1.6. */
MOZ_ALWAYS_INLINE bool
ToUint32(JSContext* cx, HandleValue v, uint32_t* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = uint32_t(v.toInt32());
        return true;
    }
    return js::ToUint32Slow(cx, v, out);
}

/* ES6 draft 20141224, 7.1.7. */
MOZ_ALWAYS_INLINE bool
ToInt16(JSContext *cx, JS::HandleValue v, int16_t *out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = int16_t(v.toInt32());
        return true;
    }
    return js::ToInt16Slow(cx, v, out);
}

/* ES6 draft 20141224, 7.1.8. */
MOZ_ALWAYS_INLINE bool
ToUint16(JSContext* cx, HandleValue v, uint16_t* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = uint16_t(v.toInt32());
        return true;
    }
    return js::ToUint16Slow(cx, v, out);
}

/* ES6 draft 20141224, 7.1.9 */
MOZ_ALWAYS_INLINE bool
ToInt8(JSContext *cx, JS::HandleValue v, int8_t *out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = int8_t(v.toInt32());
        return true;
    }
    return js::ToInt8Slow(cx, v, out);
}

/* ES6 ECMA-262, 7.1.10 */
MOZ_ALWAYS_INLINE bool
ToUint8(JSContext *cx, JS::HandleValue v, uint8_t *out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = uint8_t(v.toInt32());
        return true;
    }
    return js::ToUint8Slow(cx, v, out);
}

/*
 * Non-standard, with behavior similar to that of ToInt32, except in its
 * producing an int64_t.
 */
MOZ_ALWAYS_INLINE bool
ToInt64(JSContext* cx, HandleValue v, int64_t* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = int64_t(v.toInt32());
        return true;
    }
    return js::ToInt64Slow(cx, v, out);
}

/*
 * Non-standard, with behavior similar to that of ToUint32, except in its
 * producing a uint64_t.
 */
MOZ_ALWAYS_INLINE bool
ToUint64(JSContext* cx, HandleValue v, uint64_t* out)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isInt32()) {
        *out = uint64_t(v.toInt32());
        return true;
    }
    return js::ToUint64Slow(cx, v, out);
}

/* ES6 draft 20141224, 7.1.12. */
MOZ_ALWAYS_INLINE JSString*
ToString(JSContext* cx, HandleValue v)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isString())
        return v.toString();
    return js::ToStringSlow(cx, v);
}

/* ES6 draft 20141224, 7.1.13. */
inline JSObject*
ToObject(JSContext* cx, HandleValue v)
{
    detail::AssertArgumentsAreSane(cx, v);

    if (v.isObject())
        return &v.toObject();
    return js::ToObjectSlow(cx, v, false);
}

namespace detail {

/*
 * Convert a double value to ResultType (an unsigned integral type) using
 * ECMAScript-style semantics (that is, in like manner to how ECMAScript's
 * ToInt32 converts to int32_t).
 *
 *   If d is infinite or NaN, return 0.
 *   Otherwise compute d2 = sign(d) * floor(abs(d)), and return the ResultType
 *   value congruent to d2 mod 2**(bit width of ResultType).
 *
 * The algorithm below is inspired by that found in
 * <http://trac.webkit.org/changeset/67825/trunk/JavaScriptCore/runtime/JSValue.cpp>
 * but has been generalized to all integer widths.
 */
template<typename ResultType>
inline ResultType
ToUintWidth(double d)
{
    static_assert(mozilla::IsUnsigned<ResultType>::value,
                  "ResultType must be an unsigned type");

    uint64_t bits = mozilla::BitwiseCast<uint64_t>(d);
    unsigned DoubleExponentShift = mozilla::FloatingPoint<double>::kExponentShift;

    int_fast16_t exp =
        int_fast16_t((bits & mozilla::FloatingPoint<double>::kExponentBits) >> DoubleExponentShift) -
        int_fast16_t(mozilla::FloatingPoint<double>::kExponentBias);

    if (exp < 0)
        return 0;

    uint_fast16_t exponent = mozilla::AssertedCast<uint_fast16_t>(exp);

    const size_t ResultWidth = CHAR_BIT * sizeof(ResultType);
    if (exponent >= DoubleExponentShift + ResultWidth)
        return 0;

    static_assert(sizeof(ResultType) <= sizeof(uint64_t),
                  "Left-shifting below would lose upper bits");
    ResultType result = (exponent > DoubleExponentShift)
                        ? ResultType(bits << (exponent - DoubleExponentShift))
                        : ResultType(bits >> (DoubleExponentShift - exponent));

    if (exponent < ResultWidth) {
        ResultType implicitOne = ResultType(1) << exponent;
        result &= implicitOne - 1; // remove bogus bits
        result += implicitOne; // add the implicit bit
    }

    return (bits & mozilla::FloatingPoint<double>::kSignBit) ? ~result + 1 : result;
}

template<typename ResultType>
inline ResultType
ToIntWidth(double d)
{
    static_assert(mozilla::IsSigned<ResultType>::value,
                  "ResultType must be a signed type");

    const ResultType MaxValue = (1ULL << (CHAR_BIT * sizeof(ResultType) - 1)) - 1;
    const ResultType MinValue = -MaxValue - 1;

    typedef typename mozilla::MakeUnsigned<ResultType>::Type UnsignedResult;
    UnsignedResult u = ToUintWidth<UnsignedResult>(d);
    if (u <= UnsignedResult(MaxValue))
        return static_cast<ResultType>(u);
    return (MinValue + static_cast<ResultType>(u - MaxValue)) - 1;
}

} // namespace detail

/* ES5 9.5 ToInt32 (specialized for doubles). */
inline int32_t
ToInt32(double d)
{
#if defined (__arm__) && defined (__GNUC__) && !defined(__clang__)
    int32_t i;
    uint32_t    tmp0;
    uint32_t    tmp1;
    uint32_t    tmp2;
    asm (


"   mov     %1, %R4, LSR #20\n"
"   bic     %1, %1, #(1 << 11)\n"  // Clear the sign.

"   orr     %R4, %R4, #(1 << 20)\n"


"   sub     %1, %1, #0xff\n"
"   subs    %1, %1, #0x300\n"
"   bmi     8f\n"


"   subs    %3, %1, #52\n"         // Calculate exp-52
"   bmi     1f\n"

"   bic     %2, %3, #0xff\n"
"   orr     %3, %3, %2, LSR #3\n"
"   mov     %Q4, %Q4, LSL %3\n"
"   b       2f\n"
"1:\n" // Shift r0 right by 52-exp.
"   rsb     %3, %1, #52\n"
"   mov     %Q4, %Q4, LSR %3\n"


"2:\n"
"   subs    %3, %1, #31\n"          // Calculate exp-31
"   mov     %1, %R4, LSL #11\n"     // Re-use %1 as a temporary register.
"   bmi     3f\n"

"   bic     %2, %3, #0xff\n"
"   orr     %3, %3, %2, LSR #3\n"
"   mov     %2, %1, LSL %3\n"
"   b       4f\n"
"3:\n" // Shift r1 right by 31-exp.
"   rsb     %3, %3, #0\n"          // Calculate 31-exp from -(exp-31)
"   mov     %2, %1, LSR %3\n"      // Thumb-2 can't do "LSR %3" in "orr".


"4:\n"
"   orr     %Q4, %Q4, %2\n"
"   eor     %Q4, %Q4, %R4, ASR #31\n"
"   add     %0, %Q4, %R4, LSR #31\n"
"   b       9f\n"
"8:\n"
"   mov     %0, #0\n"
"9:\n"
    : "=r" (i), "=&r" (tmp0), "=&r" (tmp1), "=&r" (tmp2), "=&r" (d)
    : "4" (d)
    : "cc"
        );
    return i;
#else
    return detail::ToIntWidth<int32_t>(d);
#endif
}

/* ES5 9.6 (specialized for doubles). */
inline uint32_t
ToUint32(double d)
{
    return detail::ToUintWidth<uint32_t>(d);
}

/* WEBIDL 4.2.4 */
inline int8_t
ToInt8(double d)
{
    return detail::ToIntWidth<int8_t>(d);
}

/* ECMA-262 7.1.10 ToUInt8() specialized for doubles. */
inline int8_t
ToUint8(double d)
{
    return detail::ToUintWidth<uint8_t>(d);
}

/* WEBIDL 4.2.6 */
inline int16_t
ToInt16(double d)
{
    return detail::ToIntWidth<int16_t>(d);
}

inline uint16_t
ToUint16(double d)
{
    return detail::ToUintWidth<uint16_t>(d);
}

/* WEBIDL 4.2.10 */
inline int64_t
ToInt64(double d)
{
    return detail::ToIntWidth<int64_t>(d);
}

/* WEBIDL 4.2.11 */
inline uint64_t
ToUint64(double d)
{
    return detail::ToUintWidth<uint64_t>(d);
}

} // namespace JS

#endif /* js_Conversions_h */

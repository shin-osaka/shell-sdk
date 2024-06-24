
#ifndef V8CONFIG_H_
#define V8CONFIG_H_


#if defined(__ANDROID__)
# include <sys/cdefs.h>
#elif defined(__APPLE__)
# include <TargetConditionals.h>
#elif defined(__linux__)
# include <features.h>
#endif


#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
# define V8_GLIBC_PREREQ(major, minor)                                    \
    ((__GLIBC__ * 100 + __GLIBC_MINOR__) >= ((major) * 100 + (minor)))
#else
# define V8_GLIBC_PREREQ(major, minor) 0
#endif


#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define V8_GNUC_PREREQ(major, minor, patchlevel)                         \
    ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >=   \
     ((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define V8_GNUC_PREREQ(major, minor, patchlevel)      \
    ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >=      \
     ((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define V8_GNUC_PREREQ(major, minor, patchlevel) 0
#endif




#if defined(__ANDROID__)
# define V8_OS_ANDROID 1
# define V8_OS_LINUX 1
# define V8_OS_POSIX 1
#elif defined(__APPLE__)
# define V8_OS_BSD 1
# define V8_OS_MACOSX 1
# define V8_OS_POSIX 1
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#  define V8_OS_IOS 1
# endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(__CYGWIN__)
# define V8_OS_CYGWIN 1
# define V8_OS_POSIX 1
#elif defined(__linux__)
# define V8_OS_LINUX 1
# define V8_OS_POSIX 1
#elif defined(__sun)
# define V8_OS_POSIX 1
# define V8_OS_SOLARIS 1
#elif defined(_AIX)
#define V8_OS_POSIX 1
#define V8_OS_AIX 1
#elif defined(__FreeBSD__)
# define V8_OS_BSD 1
# define V8_OS_FREEBSD 1
# define V8_OS_POSIX 1
#elif defined(__Fuchsia__)
# define V8_OS_FUCHSIA 1
# define V8_OS_POSIX 1
#elif defined(__DragonFly__)
# define V8_OS_BSD 1
# define V8_OS_DRAGONFLYBSD 1
# define V8_OS_POSIX 1
#elif defined(__NetBSD__)
# define V8_OS_BSD 1
# define V8_OS_NETBSD 1
# define V8_OS_POSIX 1
#elif defined(__OpenBSD__)
# define V8_OS_BSD 1
# define V8_OS_OPENBSD 1
# define V8_OS_POSIX 1
#elif defined(__QNXNTO__)
# define V8_OS_POSIX 1
# define V8_OS_QNX 1
#elif defined(_WIN32)
# define V8_OS_WIN 1
#endif



#if defined (_MSC_VER)
# define V8_LIBC_MSVCRT 1
#elif defined(__BIONIC__)
# define V8_LIBC_BIONIC 1
# define V8_LIBC_BSD 1
#elif defined(__UCLIBC__)
# define V8_LIBC_UCLIBC 1
#elif defined(__GLIBC__) || defined(__GNU_LIBRARY__)
# define V8_LIBC_GLIBC 1
#else
# define V8_LIBC_BSD V8_OS_BSD
#endif



#if defined(__clang__)

#if defined(__GNUC__)  // Clang in gcc mode.
# define V8_CC_GNU 1
#endif

# define V8_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define V8_HAS_ATTRIBUTE_DEPRECATED (__has_attribute(deprecated))
# define V8_HAS_ATTRIBUTE_DEPRECATED_MESSAGE \
    (__has_extension(attribute_deprecated_with_message))
# define V8_HAS_ATTRIBUTE_NOINLINE (__has_attribute(noinline))
# define V8_HAS_ATTRIBUTE_UNUSED (__has_attribute(unused))
# define V8_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
# define V8_HAS_ATTRIBUTE_WARN_UNUSED_RESULT \
    (__has_attribute(warn_unused_result))

# define V8_HAS_BUILTIN_BSWAP16 (__has_builtin(__builtin_bswap16))
# define V8_HAS_BUILTIN_BSWAP32 (__has_builtin(__builtin_bswap32))
# define V8_HAS_BUILTIN_BSWAP64 (__has_builtin(__builtin_bswap64))
# define V8_HAS_BUILTIN_CLZ (__has_builtin(__builtin_clz))
# define V8_HAS_BUILTIN_CTZ (__has_builtin(__builtin_ctz))
# define V8_HAS_BUILTIN_EXPECT (__has_builtin(__builtin_expect))
# define V8_HAS_BUILTIN_FRAME_ADDRESS (__has_builtin(__builtin_frame_address))
# define V8_HAS_BUILTIN_POPCOUNT (__has_builtin(__builtin_popcount))
# define V8_HAS_BUILTIN_SADD_OVERFLOW (__has_builtin(__builtin_sadd_overflow))
# define V8_HAS_BUILTIN_SSUB_OVERFLOW (__has_builtin(__builtin_ssub_overflow))
# define V8_HAS_BUILTIN_UADD_OVERFLOW (__has_builtin(__builtin_uadd_overflow))

# if __cplusplus >= 201402L
#  define V8_CAN_HAVE_DCHECK_IN_CONSTEXPR 1
# endif

#elif defined(__GNUC__)

# define V8_CC_GNU 1
# if defined(__INTEL_COMPILER)  // Intel C++ also masquerades as GCC 3.2.0
#  define V8_CC_INTEL 1
# endif
# if defined(__MINGW32__)
#  define V8_CC_MINGW32 1
# endif
# if defined(__MINGW64__)
#  define V8_CC_MINGW64 1
# endif
# define V8_CC_MINGW (V8_CC_MINGW32 || V8_CC_MINGW64)

# define V8_HAS_ATTRIBUTE_ALWAYS_INLINE (V8_GNUC_PREREQ(4, 4, 0))
# define V8_HAS_ATTRIBUTE_DEPRECATED (V8_GNUC_PREREQ(3, 4, 0))
# define V8_HAS_ATTRIBUTE_DEPRECATED_MESSAGE (V8_GNUC_PREREQ(4, 5, 0))
# define V8_HAS_ATTRIBUTE_NOINLINE (V8_GNUC_PREREQ(3, 4, 0))
# define V8_HAS_ATTRIBUTE_UNUSED (V8_GNUC_PREREQ(2, 95, 0))
# define V8_HAS_ATTRIBUTE_VISIBILITY (V8_GNUC_PREREQ(4, 3, 0))
# define V8_HAS_ATTRIBUTE_WARN_UNUSED_RESULT \
    (!V8_CC_INTEL && V8_GNUC_PREREQ(4, 1, 0))

# define V8_HAS_BUILTIN_CLZ (V8_GNUC_PREREQ(3, 4, 0))
# define V8_HAS_BUILTIN_CTZ (V8_GNUC_PREREQ(3, 4, 0))
# define V8_HAS_BUILTIN_EXPECT (V8_GNUC_PREREQ(2, 96, 0))
# define V8_HAS_BUILTIN_FRAME_ADDRESS (V8_GNUC_PREREQ(2, 96, 0))
# define V8_HAS_BUILTIN_POPCOUNT (V8_GNUC_PREREQ(3, 4, 0))

#endif

#if defined(_MSC_VER)
# define V8_CC_MSVC 1

# define V8_HAS_DECLSPEC_DEPRECATED 1
# define V8_HAS_DECLSPEC_NOINLINE 1
# define V8_HAS_DECLSPEC_SELECTANY 1
# define V8_HAS_DECLSPEC_NORETURN 1

# define V8_HAS___FORCEINLINE 1

#endif



#if !defined(DEBUG) && V8_HAS_ATTRIBUTE_ALWAYS_INLINE
# define V8_INLINE inline __attribute__((always_inline))
#elif !defined(DEBUG) && V8_HAS___FORCEINLINE
# define V8_INLINE __forceinline
#else
# define V8_INLINE inline
#endif


#if !defined(DEBUG) && V8_HAS_ATTRIBUTE_NOINLINE
# define V8_NOINLINE __attribute__((noinline))
#elif !defined(DEBUG) && V8_HAS_DECLSPEC_NOINLINE
# define V8_NOINLINE __declspec(noinline)
#else
# define V8_NOINLINE /* NOT SUPPORTED */
#endif


#if defined(V8_DEPRECATION_WARNINGS) && V8_HAS_ATTRIBUTE_DEPRECATED_MESSAGE
#define V8_DEPRECATED(message, declarator) \
  declarator __attribute__((deprecated(message)))
#elif defined(V8_DEPRECATION_WARNINGS) && V8_HAS_ATTRIBUTE_DEPRECATED
#define V8_DEPRECATED(message, declarator) \
  declarator __attribute__((deprecated))
#elif defined(V8_DEPRECATION_WARNINGS) && V8_HAS_DECLSPEC_DEPRECATED
#define V8_DEPRECATED(message, declarator) __declspec(deprecated) declarator
#else
#define V8_DEPRECATED(message, declarator) declarator
#endif


#if defined(V8_IMMINENT_DEPRECATION_WARNINGS) && \
    V8_HAS_ATTRIBUTE_DEPRECATED_MESSAGE
#define V8_DEPRECATE_SOON(message, declarator) \
  declarator __attribute__((deprecated(message)))
#elif defined(V8_IMMINENT_DEPRECATION_WARNINGS) && V8_HAS_ATTRIBUTE_DEPRECATED
#define V8_DEPRECATE_SOON(message, declarator) \
  declarator __attribute__((deprecated))
#elif defined(V8_IMMINENT_DEPRECATION_WARNINGS) && V8_HAS_DECLSPEC_DEPRECATED
#define V8_DEPRECATE_SOON(message, declarator) __declspec(deprecated) declarator
#else
#define V8_DEPRECATE_SOON(message, declarator) declarator
#endif


#if V8_HAS_BUILTIN_EXPECT
# define V8_UNLIKELY(condition) (__builtin_expect(!!(condition), 0))
# define V8_LIKELY(condition) (__builtin_expect(!!(condition), 1))
#else
# define V8_UNLIKELY(condition) (condition)
# define V8_LIKELY(condition) (condition)
#endif


#if V8_HAS_ATTRIBUTE_WARN_UNUSED_RESULT
#define V8_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define V8_WARN_UNUSED_RESULT /* NOT SUPPORTED */
#endif

#ifdef V8_OS_WIN

#ifdef BUILDING_V8_SHARED
# define V8_EXPORT __declspec(dllexport)
#elif USING_V8_SHARED
# define V8_EXPORT __declspec(dllimport)
#else
# define V8_EXPORT
#endif  // BUILDING_V8_SHARED

#else  // V8_OS_WIN

#if V8_HAS_ATTRIBUTE_VISIBILITY
# ifdef BUILDING_V8_SHARED
#  define V8_EXPORT __attribute__ ((visibility("default")))
# else
#  define V8_EXPORT
# endif
#else
# define V8_EXPORT
#endif

#endif  // V8_OS_WIN


#endif  // V8CONFIG_H_

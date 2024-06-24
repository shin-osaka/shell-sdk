
#ifndef WEBP_WEBP_TYPES_H_
#define WEBP_WEBP_TYPES_H_

#include <stddef.h>  // for size_t

#ifndef _MSC_VER
#include <inttypes.h>
#if defined(__cplusplus) || !defined(__STRICT_ANSI__) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define WEBP_INLINE inline
#else
#define WEBP_INLINE
#endif
#else
typedef signed   char int8_t;
typedef unsigned char uint8_t;
typedef signed   short int16_t;
typedef unsigned short uint16_t;
typedef signed   int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef long long int int64_t;
#define WEBP_INLINE __forceinline
#endif  /* _MSC_VER */

#ifndef WEBP_EXTERN
# if defined(__GNUC__) && __GNUC__ >= 4
#  define WEBP_EXTERN(type) extern __attribute__ ((visibility ("default"))) type
# else
#  define WEBP_EXTERN(type) extern type
# endif  /* __GNUC__ >= 4 */
#endif  /* WEBP_EXTERN */

#define WEBP_ABI_IS_INCOMPATIBLE(a, b) (((a) >> 8) != ((b) >> 8))

#endif  /* WEBP_WEBP_TYPES_H_ */

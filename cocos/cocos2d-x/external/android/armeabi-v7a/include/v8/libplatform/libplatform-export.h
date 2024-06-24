
#ifndef V8_LIBPLATFORM_LIBPLATFORM_EXPORT_H_
#define V8_LIBPLATFORM_LIBPLATFORM_EXPORT_H_

#if defined(_WIN32)

#ifdef BUILDING_V8_PLATFORM_SHARED
#define V8_PLATFORM_EXPORT __declspec(dllexport)
#elif USING_V8_PLATFORM_SHARED
#define V8_PLATFORM_EXPORT __declspec(dllimport)
#else
#define V8_PLATFORM_EXPORT
#endif  // BUILDING_V8_PLATFORM_SHARED

#else  // defined(_WIN32)

#ifdef BUILDING_V8_PLATFORM_SHARED
#define V8_PLATFORM_EXPORT __attribute__((visibility("default")))
#else
#define V8_PLATFORM_EXPORT
#endif

#endif  // defined(_WIN32)

#endif  // V8_LIBPLATFORM_LIBPLATFORM_EXPORT_H_

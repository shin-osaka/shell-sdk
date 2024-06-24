
#ifndef GOOGLE_BREAKPAD_COMMON_MINIDUMP_TYPE_HELPER_H_
#define GOOGLE_BREAKPAD_COMMON_MINIDUMP_TYPE_HELPER_H_

#include <stdint.h>

#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

template <size_t>
struct MDTypeHelper;

template <>
struct MDTypeHelper<sizeof(uint32_t)> {
  typedef MDRawDebug32 MDRawDebug;
  typedef MDRawLinkMap32 MDRawLinkMap;
};

template <>
struct MDTypeHelper<sizeof(uint64_t)> {
  typedef MDRawDebug64 MDRawDebug;
  typedef MDRawLinkMap64 MDRawLinkMap;
};

}  // namespace google_breakpad

#endif  // GOOGLE_BREAKPAD_COMMON_MINIDUMP_TYPE_HELPER_H_

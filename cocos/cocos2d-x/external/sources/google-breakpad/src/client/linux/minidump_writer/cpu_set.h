
#ifndef CLIENT_LINUX_MINIDUMP_WRITER_CPU_SET_H_
#define CLIENT_LINUX_MINIDUMP_WRITER_CPU_SET_H_

#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "common/linux/linux_libc_support.h"
#include "third_party/lss/linux_syscall_support.h"

namespace google_breakpad {

class CpuSet {
public:
  static const size_t kMaxCpus = 1024;

  CpuSet() {
    my_memset(mask_, 0, sizeof(mask_));
  }

  bool ParseSysFile(int fd) {
    char buffer[512];
    int ret = sys_read(fd, buffer, sizeof(buffer)-1);
    if (ret < 0)
      return false;

    buffer[ret] = '\0';

    const char* p = buffer;
    const char* p_end = p + ret;
    while (p < p_end) {
      while (p < p_end && my_isspace(*p))
        p++;

      const char* item = p;
      size_t item_len = static_cast<size_t>(p_end - p);
      const char* item_next =
          static_cast<const char*>(my_memchr(p, ',', item_len));
      if (item_next != NULL) {
        p = item_next + 1;
        item_len = static_cast<size_t>(item_next - item);
      } else {
        p = p_end;
        item_next = p_end;
      }

      while (item_next > item && my_isspace(item_next[-1]))
        item_next--;

      if (item_next == item)
        continue;

      uintptr_t start = 0;
      const char* next = my_read_decimal_ptr(&start, item);
      uintptr_t end = start;
      if (*next == '-')
        my_read_decimal_ptr(&end, next+1);

      while (start <= end)
        SetBit(start++);
    }
    return true;
  }

  void IntersectWith(const CpuSet& other) {
    for (size_t nn = 0; nn < kMaskWordCount; ++nn)
      mask_[nn] &= other.mask_[nn];
  }

  int GetCount() {
    int result = 0;
    for (size_t nn = 0; nn < kMaskWordCount; ++nn) {
      result += __builtin_popcount(mask_[nn]);
    }
    return result;
  }

private:
  void SetBit(uintptr_t index) {
    size_t nn = static_cast<size_t>(index);
    if (nn < kMaxCpus)
      mask_[nn / kMaskWordBits] |= (1U << (nn % kMaskWordBits));
  }

  typedef uint32_t MaskWordType;
  static const size_t kMaskWordBits = 8*sizeof(MaskWordType);
  static const size_t kMaskWordCount =
      (kMaxCpus + kMaskWordBits - 1) / kMaskWordBits;

  MaskWordType mask_[kMaskWordCount];
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_MINIDUMP_WRITER_CPU_SET_H_

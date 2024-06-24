
#ifndef CLIENT_LINUX_DUMP_WRITER_COMMON_UCONTEXT_READER_H
#define CLIENT_LINUX_DUMP_WRITER_COMMON_UCONTEXT_READER_H

#include <sys/ucontext.h>
#include <sys/user.h>

#include "client/linux/dump_writer_common/raw_context_cpu.h"
#include "common/memory.h"
#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

struct UContextReader {
  static uintptr_t GetStackPointer(const struct ucontext* uc);

  static uintptr_t GetInstructionPointer(const struct ucontext* uc);

#if defined(__i386__) || defined(__x86_64)
  static void FillCPUContext(RawContextCPU *out, const ucontext *uc,
                             const struct _libc_fpstate* fp);
#elif defined(__aarch64__)
  static void FillCPUContext(RawContextCPU *out, const ucontext *uc,
                             const struct fpsimd_context* fpregs);
#else
  static void FillCPUContext(RawContextCPU *out, const ucontext *uc);
#endif
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_DUMP_WRITER_COMMON_UCONTEXT_READER_H

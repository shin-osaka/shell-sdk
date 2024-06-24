
#ifndef CLIENT_LINUX_DUMP_WRITER_COMMON_THREAD_INFO_H_
#define CLIENT_LINUX_DUMP_WRITER_COMMON_THREAD_INFO_H_

#include <sys/ucontext.h>
#include <sys/user.h>

#include "client/linux/dump_writer_common/raw_context_cpu.h"
#include "common/memory.h"
#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

#if defined(__i386) || defined(__x86_64)
typedef __typeof__(((struct user*) 0)->u_debugreg[0]) debugreg_t;
#endif

struct ThreadInfo {
  pid_t tgid;   // thread group id
  pid_t ppid;   // parent process

  uintptr_t stack_pointer;  // thread stack pointer


#if defined(__i386) || defined(__x86_64)
  user_regs_struct regs;
  user_fpregs_struct fpregs;
  static const unsigned kNumDebugRegisters = 8;
  debugreg_t dregs[8];
#if defined(__i386)
  user_fpxregs_struct fpxregs;
#endif  // defined(__i386)

#elif defined(__ARM_EABI__)
  struct user_regs regs;
  struct user_fpregs fpregs;
#elif defined(__aarch64__)
  struct user_regs_struct regs;
  struct user_fpsimd_struct fpregs;
#elif defined(__mips__)
  mcontext_t mcontext;
#endif

  uintptr_t GetInstructionPointer() const;

  void FillCPUContext(RawContextCPU* out) const;

  void GetGeneralPurposeRegisters(void** gp_regs, size_t* size);

  void GetFloatingPointRegisters(void** fp_regs, size_t* size);
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_DUMP_WRITER_COMMON_THREAD_INFO_H_


#ifndef CLIENT_LINUX_DUMP_WRITER_COMMON_RAW_CONTEXT_CPU_H
#define CLIENT_LINUX_DUMP_WRITER_COMMON_RAW_CONTEXT_CPU_H

#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

#if defined(__i386__)
typedef MDRawContextX86 RawContextCPU;
#elif defined(__x86_64)
typedef MDRawContextAMD64 RawContextCPU;
#elif defined(__ARM_EABI__)
typedef MDRawContextARM RawContextCPU;
#elif defined(__aarch64__)
typedef MDRawContextARM64 RawContextCPU;
#elif defined(__mips__)
typedef MDRawContextMIPS RawContextCPU;
#else
#error "This code has not been ported to your platform yet."
#endif

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_DUMP_WRITER_COMMON_RAW_CONTEXT_CPU_H

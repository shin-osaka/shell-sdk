
#ifndef CLIENT_LINUX_CRASH_GENERATION_CRASH_GENERATION_CLIENT_H_
#define CLIENT_LINUX_CRASH_GENERATION_CRASH_GENERATION_CLIENT_H_

#include "common/basictypes.h"

#include <stddef.h>

namespace google_breakpad {

class CrashGenerationClient {
 public:
  CrashGenerationClient() {}
  virtual ~CrashGenerationClient() {}

  virtual bool RequestDump(const void* blob, size_t blob_size) = 0;

  static CrashGenerationClient* TryCreate(int server_fd);

 private:
  DISALLOW_COPY_AND_ASSIGN(CrashGenerationClient);
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_CRASH_GENERATION_CRASH_GENERATION_CLIENT_H_

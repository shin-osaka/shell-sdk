
#ifndef CLIENT_LINUX_LOG_LOG_H_
#define CLIENT_LINUX_LOG_LOG_H_

#include <stddef.h>

namespace logger {

int write(const char* buf, size_t nbytes);

#if defined(__ANDROID__)

void initializeCrashLogWriter();

int writeToCrashLog(const char* buf);
#endif

}  // namespace logger

#endif  // CLIENT_LINUX_LOG_LOG_H_

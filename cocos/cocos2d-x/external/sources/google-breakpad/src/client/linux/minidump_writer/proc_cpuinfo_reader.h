
#ifndef CLIENT_LINUX_MINIDUMP_WRITER_PROC_CPUINFO_READER_H_
#define CLIENT_LINUX_MINIDUMP_WRITER_PROC_CPUINFO_READER_H_

#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "client/linux/minidump_writer/line_reader.h"
#include "common/linux/linux_libc_support.h"
#include "third_party/lss/linux_syscall_support.h"

namespace google_breakpad {

class ProcCpuInfoReader {
public:
  ProcCpuInfoReader(int fd)
    : line_reader_(fd), pop_count_(-1) {
  }

  bool GetNextField(const char** field) {
    for (;;) {
      const char* line;
      unsigned line_len;

      if (pop_count_ >= 0) {
        line_reader_.PopLine(pop_count_);
        pop_count_ = -1;
      }

      if (!line_reader_.GetNextLine(&line, &line_len))
        return false;

      pop_count_ = static_cast<int>(line_len);

      const char* line_end = line + line_len;

      char* sep = static_cast<char*>(my_memchr(line, ':', line_len));
      if (sep == NULL)
        continue;

      const char* val = sep+1;
      while (val < line_end && my_isspace(*val))
        val++;

      value_ = val;
      value_len_ = static_cast<size_t>(line_end - val);

      while (sep > line && my_isspace(sep[-1]))
        sep--;

      if (sep == line)
        continue;

      *sep = '\0';

      *field = line;
      return true;
    }
  }

  const char* GetValue() {
    assert(value_);
    return value_;
  }

  const char* GetValueAndLen(size_t* length) {
    assert(value_);
    *length = value_len_;
    return value_;
  }

private:
  LineReader line_reader_;
  int pop_count_;
  const char* value_;
  size_t value_len_;
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_MINIDUMP_WRITER_PROC_CPUINFO_READER_H_

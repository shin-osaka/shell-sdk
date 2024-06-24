
#ifndef CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_
#define CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_

#include <assert.h>
#include <sys/types.h>

#include <string>

#include "client/linux/handler/microdump_extra_info.h"
#include "common/using_std_string.h"

namespace google_breakpad {

class MinidumpDescriptor {
 public:
  struct MicrodumpOnConsole {};
  static const MicrodumpOnConsole kMicrodumpOnConsole;

  MinidumpDescriptor()
      : mode_(kUninitialized),
        fd_(-1),
        size_limit_(-1) {}

  explicit MinidumpDescriptor(const string& directory)
      : mode_(kWriteMinidumpToFile),
        fd_(-1),
        directory_(directory),
        c_path_(NULL),
        size_limit_(-1) {
    assert(!directory.empty());
  }

  explicit MinidumpDescriptor(int fd)
      : mode_(kWriteMinidumpToFd),
        fd_(fd),
        c_path_(NULL),
        size_limit_(-1) {
    assert(fd != -1);
  }

  explicit MinidumpDescriptor(const MicrodumpOnConsole&)
      : mode_(kWriteMicrodumpToConsole),
        fd_(-1),
        size_limit_(-1) {}

  explicit MinidumpDescriptor(const MinidumpDescriptor& descriptor);
  MinidumpDescriptor& operator=(const MinidumpDescriptor& descriptor);

  static MinidumpDescriptor getMicrodumpDescriptor();

  bool IsFD() const { return mode_ == kWriteMinidumpToFd; }

  int fd() const { return fd_; }

  string directory() const { return directory_; }

  const char* path() const { return c_path_; }

  bool IsMicrodumpOnConsole() const {
    return mode_ == kWriteMicrodumpToConsole;
  }

  void UpdatePath();

  off_t size_limit() const { return size_limit_; }
  void set_size_limit(off_t limit) { size_limit_ = limit; }

  MicrodumpExtraInfo* microdump_extra_info() {
    assert(IsMicrodumpOnConsole());
    return &microdump_extra_info_;
  };

 private:
  enum DumpMode {
    kUninitialized = 0,
    kWriteMinidumpToFile,
    kWriteMinidumpToFd,
    kWriteMicrodumpToConsole
  };

  DumpMode mode_;

  int fd_;

  string directory_;

  string path_;

  const char* c_path_;

  off_t size_limit_;

  MicrodumpExtraInfo microdump_extra_info_;
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_HANDLER_MINIDUMP_DESCRIPTOR_H_

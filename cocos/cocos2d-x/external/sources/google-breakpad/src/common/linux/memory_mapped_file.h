

#ifndef COMMON_LINUX_MEMORY_MAPPED_FILE_H_
#define COMMON_LINUX_MEMORY_MAPPED_FILE_H_

#include <stddef.h>
#include "common/basictypes.h"
#include "common/memory_range.h"

namespace google_breakpad {

class MemoryMappedFile {
 public:
  MemoryMappedFile();

  MemoryMappedFile(const char* path, size_t offset);

  ~MemoryMappedFile();

  bool Map(const char* path, size_t offset);

  void Unmap();

  const MemoryRange& content() const { return content_; }

  const void* data() const { return content_.data(); }

  size_t size() const { return content_.length(); }

 private:
  MemoryRange content_;

  DISALLOW_COPY_AND_ASSIGN(MemoryMappedFile);
};

}  // namespace google_breakpad

#endif  // COMMON_LINUX_MEMORY_MAPPED_FILE_H_

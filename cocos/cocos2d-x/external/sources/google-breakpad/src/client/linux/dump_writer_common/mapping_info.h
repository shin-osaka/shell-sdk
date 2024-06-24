
#ifndef CLIENT_LINUX_DUMP_WRITER_COMMON_MAPPING_INFO_H_
#define CLIENT_LINUX_DUMP_WRITER_COMMON_MAPPING_INFO_H_

#include <limits.h>
#include <list>
#include <stdint.h>

#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

struct MappingInfo {
  uintptr_t start_addr;
  size_t size;
  size_t offset;  // offset into the backed file.
  bool exec;  // true if the mapping has the execute bit set.
  char name[NAME_MAX];
};

struct MappingEntry {
  MappingInfo first;
  uint8_t second[sizeof(MDGUID)];
};

typedef std::list<MappingEntry> MappingList;

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_DUMP_WRITER_COMMON_MAPPING_INFO_H_

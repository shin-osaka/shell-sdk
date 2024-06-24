
#ifndef CLIENT_LINUX_MINIDUMP_WRITER_MICRODUMP_WRITER_H_
#define CLIENT_LINUX_MINIDUMP_WRITER_MICRODUMP_WRITER_H_

#include <stdint.h>
#include <sys/types.h>

#include "client/linux/dump_writer_common/mapping_info.h"

namespace google_breakpad {

struct MicrodumpExtraInfo;

bool WriteMicrodump(pid_t crashing_process,
                    const void* blob,
                    size_t blob_size,
                    const MappingList& mappings,
                    const MicrodumpExtraInfo& microdump_extra_info);

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_MINIDUMP_WRITER_MICRODUMP_WRITER_H_

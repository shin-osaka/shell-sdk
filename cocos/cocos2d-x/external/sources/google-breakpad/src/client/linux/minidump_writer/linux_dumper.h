

#ifndef CLIENT_LINUX_MINIDUMP_WRITER_LINUX_DUMPER_H_
#define CLIENT_LINUX_MINIDUMP_WRITER_LINUX_DUMPER_H_

#include <elf.h>
#if defined(__ANDROID__)
#include <link.h>
#endif
#include <linux/limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/user.h>

#include "client/linux/dump_writer_common/mapping_info.h"
#include "client/linux/dump_writer_common/thread_info.h"
#include "common/memory.h"
#include "google_breakpad/common/minidump_format.h"

namespace google_breakpad {

#if defined(__i386) || defined(__ARM_EABI__) || \
 (defined(__mips__) && _MIPS_SIM == _ABIO32)
typedef Elf32_auxv_t elf_aux_entry;
#elif defined(__x86_64) || defined(__aarch64__) || \
     (defined(__mips__) && _MIPS_SIM != _ABIO32)
typedef Elf64_auxv_t elf_aux_entry;
#endif

typedef __typeof__(((elf_aux_entry*) 0)->a_un.a_val) elf_aux_val_t;

const char kLinuxGateLibraryName[] = "linux-gate.so";

class LinuxDumper {
 public:
  explicit LinuxDumper(pid_t pid, const char* root_prefix = "");

  virtual ~LinuxDumper();

  virtual bool Init();

  virtual bool LateInit();

  virtual bool IsPostMortem() const = 0;

  virtual bool ThreadsSuspend() = 0;
  virtual bool ThreadsResume() = 0;

  virtual bool GetThreadInfoByIndex(size_t index, ThreadInfo* info) = 0;

  const wasteful_vector<pid_t> &threads() { return threads_; }
  const wasteful_vector<MappingInfo*> &mappings() { return mappings_; }
  const MappingInfo* FindMapping(const void* address) const;
  const wasteful_vector<elf_aux_val_t>& auxv() { return auxv_; }

  bool GetStackInfo(const void** stack, size_t* stack_len, uintptr_t stack_top);

  PageAllocator* allocator() { return &allocator_; }

  virtual bool CopyFromProcess(void* dest, pid_t child, const void* src,
                               size_t length) = 0;

  virtual bool BuildProcPath(char* path, pid_t pid, const char* node) const = 0;

  bool ElfFileIdentifierForMapping(const MappingInfo& mapping,
                                   bool member,
                                   unsigned int mapping_id,
                                   uint8_t identifier[sizeof(MDGUID)]);

  uintptr_t crash_address() const { return crash_address_; }
  void set_crash_address(uintptr_t crash_address) {
    crash_address_ = crash_address;
  }

  int crash_signal() const { return crash_signal_; }
  void set_crash_signal(int crash_signal) { crash_signal_ = crash_signal; }

  pid_t crash_thread() const { return crash_thread_; }
  void set_crash_thread(pid_t crash_thread) { crash_thread_ = crash_thread; }

  bool GetMappingAbsolutePath(const MappingInfo& mapping,
                              char path[PATH_MAX]) const;

  void GetMappingEffectiveNameAndPath(const MappingInfo& mapping,
                                      char* file_path,
                                      size_t file_path_size,
                                      char* file_name,
                                      size_t file_name_size);

 protected:
  bool ReadAuxv();

  virtual bool EnumerateMappings();

  virtual bool EnumerateThreads() = 0;

  bool HandleDeletedFileInMapping(char* path) const;

  const pid_t pid_;

  const char* const root_prefix_;

  uintptr_t crash_address_;

  int crash_signal_;

  pid_t crash_thread_;

  mutable PageAllocator allocator_;

  wasteful_vector<pid_t> threads_;

  wasteful_vector<MappingInfo*> mappings_;

  wasteful_vector<elf_aux_val_t> auxv_;

#if defined(__ANDROID__)
 private:

  bool GetLoadedElfHeader(uintptr_t start_addr, ElfW(Ehdr)* ehdr);

  void ParseLoadedElfProgramHeaders(ElfW(Ehdr)* ehdr,
                                    uintptr_t start_addr,
                                    uintptr_t* min_vaddr_ptr,
                                    uintptr_t* dyn_vaddr_ptr,
                                    size_t* dyn_count_ptr);

  bool HasAndroidPackedRelocations(uintptr_t load_bias,
                                   uintptr_t dyn_vaddr,
                                   size_t dyn_count);

  uintptr_t GetEffectiveLoadBias(ElfW(Ehdr)* ehdr, uintptr_t start_addr);

  void LatePostprocessMappings();
#endif  // __ANDROID__
};

}  // namespace google_breakpad

#endif  // CLIENT_LINUX_HANDLER_LINUX_DUMPER_H_

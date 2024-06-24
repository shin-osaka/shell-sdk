
#ifndef GOOGLE_BREAKPAD_ANDROID_INCLUDE_LINK_H
#define GOOGLE_BREAKPAD_ANDROID_INCLUDE_LINK_H

/* Android doesn't provide all the data-structures required in its <link.h>.
   Provide custom version here. */
#include_next <link.h>

#if !defined(__aarch64__) && !defined(__x86_64__) && \
    !(defined(__mips__) && _MIPS_SIM == _ABI64)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct r_debug {
  int              r_version;
  struct link_map* r_map;
  ElfW(Addr)       r_brk;
  enum {
    RT_CONSISTENT,
    RT_ADD,
    RT_DELETE }    r_state;
  ElfW(Addr)       r_ldbase;
};

struct link_map {
  ElfW(Addr)       l_addr;
  char*            l_name;
  ElfW(Dyn)*       l_ld;
  struct link_map* l_next;
  struct link_map* l_prev;
};

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // !defined(__aarch64__) && !defined(__x86_64__)

#endif /* GOOGLE_BREAKPAD_ANDROID_INCLUDE_LINK_H */

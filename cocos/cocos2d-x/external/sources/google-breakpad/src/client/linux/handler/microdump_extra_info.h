
#ifndef CLIENT_LINUX_HANDLER_MICRODUMP_EXTRA_INFO_H_
#define CLIENT_LINUX_HANDLER_MICRODUMP_EXTRA_INFO_H_

namespace google_breakpad {

struct MicrodumpExtraInfo {
  const char* build_fingerprint;
  const char* product_info;
  const char* gpu_fingerprint;

  MicrodumpExtraInfo()
      : build_fingerprint(NULL), product_info(NULL), gpu_fingerprint(NULL) {}
};

}

#endif  // CLIENT_LINUX_HANDLER_MICRODUMP_EXTRA_INFO_H_

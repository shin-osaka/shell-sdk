
#ifndef GOOGLE_BREAKPAD_COMMON_ANDROID_INCLUDE_SYS_USER_H
#define GOOGLE_BREAKPAD_COMMON_ANDROID_INCLUDE_SYS_USER_H



#if __ANDROID_API__ < 21
#include_next <sys/user.h>
#endif

#ifdef __i386__
#if __ANDROID_API__ >= 21
#include_next <sys/user.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
typedef struct user_fxsr_struct user_fpxregs_struct;
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
#endif  // __i386__

#ifdef __aarch64__
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
struct user_regs_struct {
 __u64 regs[31];
 __u64 sp;
 __u64 pc;
 __u64 pstate;
};
struct user_fpsimd_struct {
 __uint128_t vregs[32];
 __u32 fpsr;
 __u32 fpcr;
};
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
#endif  // __aarch64__

#endif  // GOOGLE_BREAKPAD_COMMON_ANDROID_INCLUDE_SYS_USER_H

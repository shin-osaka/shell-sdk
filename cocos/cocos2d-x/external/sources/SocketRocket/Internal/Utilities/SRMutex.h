
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef __attribute__((capability("mutex"))) pthread_mutex_t *SRMutex;

extern SRMutex SRMutexInitRecursive(void);
extern void SRMutexDestroy(SRMutex mutex);

extern void SRMutexLock(SRMutex mutex) __attribute__((acquire_capability(mutex)));
extern void SRMutexUnlock(SRMutex mutex) __attribute__((release_capability(mutex)));

NS_ASSUME_NONNULL_END

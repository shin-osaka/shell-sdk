
#import "SRMutex.h"

#import <pthread/pthread.h>

NS_ASSUME_NONNULL_BEGIN

SRMutex SRMutexInitRecursive(void)
{
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attributes;

    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attributes);
    pthread_mutexattr_destroy(&attributes);

    return mutex;
}

void SRMutexDestroy(SRMutex mutex)
{
    pthread_mutex_destroy(mutex);
    free(mutex);
}

__attribute__((no_thread_safety_analysis))
void SRMutexLock(SRMutex mutex)
{
    pthread_mutex_lock(mutex);
}

__attribute__((no_thread_safety_analysis))
void SRMutexUnlock(SRMutex mutex)
{
    pthread_mutex_unlock(mutex);
}

NS_ASSUME_NONNULL_END

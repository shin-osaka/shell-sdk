
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface SRRunLoopThread : NSThread

@property (nonatomic, strong, readonly) NSRunLoop *runLoop;

+ (instancetype)sharedThread;

@end

NS_ASSUME_NONNULL_END


#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSRunLoop (SRWebSocket)

/**
 Default run loop that will be used to schedule all instances of `SRWebSocket`.

 @return An instance of `NSRunLoop`.
 */
+ (NSRunLoop *)SR_networkRunLoop;

@end

NS_ASSUME_NONNULL_END

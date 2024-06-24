
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void(^SRProxyConnectCompletion)(NSError *_Nullable error,
                                        NSInputStream *_Nullable readStream,
                                        NSOutputStream *_Nullable writeStream);

@interface SRProxyConnect : NSObject

- (instancetype)initWithURL:(NSURL *)url;

- (void)openNetworkStreamWithCompletion:(SRProxyConnectCompletion)completion;

@end

NS_ASSUME_NONNULL_END

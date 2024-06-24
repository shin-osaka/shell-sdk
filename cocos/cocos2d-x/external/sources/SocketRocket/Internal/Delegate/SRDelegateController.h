
#import <Foundation/Foundation.h>

#import <SocketRocket/SRWebSocket.h>

NS_ASSUME_NONNULL_BEGIN

struct SRDelegateAvailableMethods {
    BOOL didReceiveMessage;
    BOOL didReceiveMessageWithString;
    BOOL didReceiveMessageWithData;
    BOOL didOpen;
    BOOL didFailWithError;
    BOOL didCloseWithCode;
    BOOL didReceivePing;
    BOOL didReceivePong;
    BOOL shouldConvertTextFrameToString;
};
typedef struct SRDelegateAvailableMethods SRDelegateAvailableMethods;

typedef void(^SRDelegateBlock)(id<SRWebSocketDelegate> _Nullable delegate, SRDelegateAvailableMethods availableMethods);

@interface SRDelegateController : NSObject

@property (nonatomic, weak) id<SRWebSocketDelegate> delegate;
@property (atomic, readonly) SRDelegateAvailableMethods availableDelegateMethods;

@property (nullable, nonatomic, strong) dispatch_queue_t dispatchQueue;
@property (nullable, nonatomic, strong) NSOperationQueue *operationQueue;

#pragma mark - Perform

- (void)performDelegateBlock:(SRDelegateBlock)block;
- (void)performDelegateQueueBlock:(dispatch_block_t)block;

@end

NS_ASSUME_NONNULL_END

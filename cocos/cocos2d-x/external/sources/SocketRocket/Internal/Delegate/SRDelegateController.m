
#import "SRDelegateController.h"

NS_ASSUME_NONNULL_BEGIN

@interface SRDelegateController ()

@property (nonatomic, strong, readonly) dispatch_queue_t accessQueue;

@property (atomic, assign, readwrite) SRDelegateAvailableMethods availableDelegateMethods;

@end

@implementation SRDelegateController

@synthesize delegate = _delegate;
@synthesize dispatchQueue = _dispatchQueue;
@synthesize operationQueue = _operationQueue;

#pragma mark - Init

- (instancetype)init
{
    self = [super init];
    if (!self) return self;

    _accessQueue = dispatch_queue_create("com.facebook.socketrocket.delegate.access", DISPATCH_QUEUE_CONCURRENT);
    _dispatchQueue = dispatch_get_main_queue();

    return self;
}

#pragma mark - Accessors

- (void)setDelegate:(id<SRWebSocketDelegate> _Nullable)delegate
{
    dispatch_barrier_async(self.accessQueue, ^{
        _delegate = delegate;

        self.availableDelegateMethods = (SRDelegateAvailableMethods){
            .didReceiveMessage = [delegate respondsToSelector:@selector(webSocket:didReceiveMessage:)],
            .didReceiveMessageWithString = [delegate respondsToSelector:@selector(webSocket:didReceiveMessageWithString:)],
            .didReceiveMessageWithData = [delegate respondsToSelector:@selector(webSocket:didReceiveMessageWithData:)],
            .didOpen = [delegate respondsToSelector:@selector(webSocketDidOpen:)],
            .didFailWithError = [delegate respondsToSelector:@selector(webSocket:didFailWithError:)],
            .didCloseWithCode = [delegate respondsToSelector:@selector(webSocket:didCloseWithCode:reason:wasClean:)],
            .didReceivePing = [delegate respondsToSelector:@selector(webSocket:didReceivePingWithData:)],
            .didReceivePong = [delegate respondsToSelector:@selector(webSocket:didReceivePong:)],
            .shouldConvertTextFrameToString = [delegate respondsToSelector:@selector(webSocketShouldConvertTextFrameToString:)]
        };
    });
}

- (id<SRWebSocketDelegate> _Nullable)delegate
{
    __block id<SRWebSocketDelegate> delegate = nil;
    dispatch_sync(self.accessQueue, ^{
        delegate = _delegate;
    });
    return delegate;
}

- (void)setDispatchQueue:(dispatch_queue_t _Nullable)queue
{
    dispatch_barrier_async(self.accessQueue, ^{
        _dispatchQueue = queue ?: dispatch_get_main_queue();
        _operationQueue = nil;
    });
}

- (dispatch_queue_t _Nullable)dispatchQueue
{
    __block dispatch_queue_t queue = nil;
    dispatch_sync(self.accessQueue, ^{
        queue = _dispatchQueue;
    });
    return queue;
}

- (void)setOperationQueue:(NSOperationQueue *_Nullable)queue
{
    dispatch_barrier_async(self.accessQueue, ^{
        _dispatchQueue = queue ? nil : dispatch_get_main_queue();
        _operationQueue = queue;
    });
}

- (NSOperationQueue *_Nullable)operationQueue
{
    __block NSOperationQueue *queue = nil;
    dispatch_sync(self.accessQueue, ^{
        queue = _operationQueue;
    });
    return queue;
}

#pragma mark - Perform

- (void)performDelegateBlock:(SRDelegateBlock)block
{
    __block __strong id<SRWebSocketDelegate> delegate = nil;
    __block SRDelegateAvailableMethods availableMethods = {};
    dispatch_sync(self.accessQueue, ^{
        delegate = _delegate; // Not `OK` to go through `self`, since queue sync.
        availableMethods = self.availableDelegateMethods; // `OK` to call through `self`, since no queue sync.
    });
    [self performDelegateQueueBlock:^{
        block(delegate, availableMethods);
    }];
}

- (void)performDelegateQueueBlock:(dispatch_block_t)block
{
    dispatch_queue_t dispatchQueue = self.dispatchQueue;
    if (dispatchQueue) {
        dispatch_async(dispatchQueue, block);
    } else {
        [self.operationQueue addOperationWithBlock:block];
    }
}

@end

NS_ASSUME_NONNULL_END

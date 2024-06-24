
#import "SRIOConsumerPool.h"

@implementation SRIOConsumerPool {
    NSUInteger _poolSize;
    NSMutableArray<SRIOConsumer *> *_bufferedConsumers;
}

- (instancetype)initWithBufferCapacity:(NSUInteger)poolSize;
{
    self = [super init];
    if (self) {
        _poolSize = poolSize;
        _bufferedConsumers = [NSMutableArray arrayWithCapacity:poolSize];
    }
    return self;
}

- (instancetype)init
{
    return [self initWithBufferCapacity:8];
}

- (SRIOConsumer *)consumerWithScanner:(stream_scanner)scanner
                              handler:(data_callback)handler
                          bytesNeeded:(size_t)bytesNeeded
                   readToCurrentFrame:(BOOL)readToCurrentFrame
                          unmaskBytes:(BOOL)unmaskBytes
{
    SRIOConsumer *consumer = nil;
    if (_bufferedConsumers.count) {
        consumer = [_bufferedConsumers lastObject];
        [_bufferedConsumers removeLastObject];
    } else {
        consumer = [[SRIOConsumer alloc] init];
    }

    [consumer resetWithScanner:scanner
                       handler:handler
                   bytesNeeded:bytesNeeded
            readToCurrentFrame:readToCurrentFrame
                   unmaskBytes:unmaskBytes];

    return consumer;
}

- (void)returnConsumer:(SRIOConsumer *)consumer;
{
    if (_bufferedConsumers.count < _poolSize) {
        [_bufferedConsumers addObject:consumer];
    }
}

-(void)clear
{
    _poolSize = 0;
    _bufferedConsumers = nil;
}

@end

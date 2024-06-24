
#import <Foundation/Foundation.h>

#import "SRIOConsumer.h" // TODO: (nlutsenko) Convert to @class and constants file for block types

@interface SRIOConsumerPool : NSObject

- (instancetype)initWithBufferCapacity:(NSUInteger)poolSize;

- (SRIOConsumer *)consumerWithScanner:(stream_scanner)scanner
                              handler:(data_callback)handler
                          bytesNeeded:(size_t)bytesNeeded
                   readToCurrentFrame:(BOOL)readToCurrentFrame
                          unmaskBytes:(BOOL)unmaskBytes;
- (void)returnConsumer:(SRIOConsumer *)consumer;

- (void)clear;

@end

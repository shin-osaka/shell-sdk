
#import <Foundation/Foundation.h>

@class SRWebSocket; // TODO: (nlutsenko) Remove dependency on SRWebSocket here.

typedef size_t (^stream_scanner)(NSData *collected_data);
typedef void (^data_callback)(SRWebSocket *webSocket,  NSData *data);

@interface SRIOConsumer : NSObject {
    stream_scanner _scanner;
    data_callback _handler;
    size_t _bytesNeeded;
    BOOL _readToCurrentFrame;
    BOOL _unmaskBytes;
}
@property (nonatomic, copy, readonly) stream_scanner consumer;
@property (nonatomic, copy, readonly) data_callback handler;
@property (nonatomic, assign) size_t bytesNeeded;
@property (nonatomic, assign, readonly) BOOL readToCurrentFrame;
@property (nonatomic, assign, readonly) BOOL unmaskBytes;

- (void)resetWithScanner:(stream_scanner)scanner
                 handler:(data_callback)handler
             bytesNeeded:(size_t)bytesNeeded
      readToCurrentFrame:(BOOL)readToCurrentFrame
             unmaskBytes:(BOOL)unmaskBytes;

@end

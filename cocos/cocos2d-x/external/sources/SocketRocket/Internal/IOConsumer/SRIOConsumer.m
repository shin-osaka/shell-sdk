
#import "SRIOConsumer.h"

@implementation SRIOConsumer

@synthesize bytesNeeded = _bytesNeeded;
@synthesize consumer = _scanner;
@synthesize handler = _handler;
@synthesize readToCurrentFrame = _readToCurrentFrame;
@synthesize unmaskBytes = _unmaskBytes;

- (void)resetWithScanner:(stream_scanner)scanner
                 handler:(data_callback)handler
             bytesNeeded:(size_t)bytesNeeded
      readToCurrentFrame:(BOOL)readToCurrentFrame
             unmaskBytes:(BOOL)unmaskBytes
{
    _scanner = [scanner copy];
    _handler = [handler copy];
    _bytesNeeded = bytesNeeded;
    _readToCurrentFrame = readToCurrentFrame;
    _unmaskBytes = unmaskBytes;
    assert(_scanner || _bytesNeeded);
}

@end

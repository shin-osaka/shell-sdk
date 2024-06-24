
#import "NSRunLoop+SRWebSocket.h"
#import "NSRunLoop+SRWebSocketPrivate.h"

#import "SRRunLoopThread.h"

void import_NSRunLoop_SRWebSocket() { }

@implementation NSRunLoop (SRWebSocket)

+ (NSRunLoop *)SR_networkRunLoop
{
    return [SRRunLoopThread sharedThread].runLoop;
}

@end

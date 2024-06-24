
#import "SRConstants.h"

size_t SRDefaultBufferSize(void) {
    static size_t size;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        size = getpagesize();
    });
    return size;
}

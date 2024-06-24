
#import "SRRandom.h"

#import <Security/SecRandom.h>

NS_ASSUME_NONNULL_BEGIN

NSData *SRRandomData(NSUInteger length)
{
    NSMutableData *data = [NSMutableData dataWithLength:length];
    int result = SecRandomCopyBytes(kSecRandomDefault, data.length, data.mutableBytes);
    if (result != 0) {
        [NSException raise:NSInternalInconsistencyException format:@"Failed to generate random bytes with OSStatus: %d", result];
    }
    return data;
}

NS_ASSUME_NONNULL_END

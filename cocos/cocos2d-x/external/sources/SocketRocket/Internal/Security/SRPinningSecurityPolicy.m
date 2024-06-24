
#import "SRPinningSecurityPolicy.h"

#import <Foundation/Foundation.h>

#import "SRLog.h"

NS_ASSUME_NONNULL_BEGIN

@interface SRPinningSecurityPolicy ()

@property (nonatomic, copy, readonly) NSArray *pinnedCertificates;

@end

@implementation SRPinningSecurityPolicy

- (instancetype)initWithCertificates:(NSArray *)pinnedCertificates
{
    self = [super initWithCertificateChainValidationEnabled:NO];
    if (!self) { return self; }

    if (pinnedCertificates.count == 0) {
        @throw [NSException exceptionWithName:@"Creating security policy failed."
                                       reason:@"Must specify at least one certificate when creating a pinning policy."
                                     userInfo:nil];
    }
    _pinnedCertificates = [pinnedCertificates copy];

    return self;
}

- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust forDomain:(NSString *)domain
{
    SRDebugLog(@"Pinned cert count: %d", self.pinnedCertificates.count);
    NSUInteger requiredCertCount = self.pinnedCertificates.count;

    NSUInteger validatedCertCount = 0;
    CFIndex serverCertCount = SecTrustGetCertificateCount(serverTrust);
    for (CFIndex i = 0; i < serverCertCount; i++) {
        SecCertificateRef cert = SecTrustGetCertificateAtIndex(serverTrust, i);
        NSData *data = CFBridgingRelease(SecCertificateCopyData(cert));
        for (id ref in self.pinnedCertificates) {
            SecCertificateRef trustedCert = (__bridge SecCertificateRef)ref;
            NSData *trustedCertData = CFBridgingRelease(SecCertificateCopyData(trustedCert));
            if ([trustedCertData isEqualToData:data]) {
                validatedCertCount++;
                break;
            }
        }
    }
    return (requiredCertCount == validatedCertCount);
}

@end

NS_ASSUME_NONNULL_END

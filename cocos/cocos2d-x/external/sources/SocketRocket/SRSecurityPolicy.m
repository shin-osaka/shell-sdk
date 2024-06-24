
#import "SRSecurityPolicy.h"
#import "SRPinningSecurityPolicy.h"

NS_ASSUME_NONNULL_BEGIN

@interface SRSecurityPolicy ()

@property (nonatomic, assign, readonly) BOOL certificateChainValidationEnabled;

@end

@implementation SRSecurityPolicy

+ (instancetype)defaultPolicy
{
    return [self new];
}

+ (instancetype)pinnningPolicyWithCertificates:(NSArray *)pinnedCertificates
{
    return [[SRPinningSecurityPolicy alloc] initWithCertificates:pinnedCertificates];
}

- (instancetype)initWithCertificateChainValidationEnabled:(BOOL)enabled
{
    self = [super init];
    if (!self) { return self; }

    _certificateChainValidationEnabled = enabled;

    return self;
}

- (instancetype)init
{
    return [self initWithCertificateChainValidationEnabled:YES];
}

- (void)updateSecurityOptionsInStream:(NSStream *)stream
{
    [stream setProperty:(__bridge id)CFSTR("kCFStreamSocketSecurityLevelTLSv1_2") forKey:(__bridge id)kCFStreamPropertySocketSecurityLevel];

    NSDictionary<NSString *, id> *sslOptions = @{ (__bridge NSString *)kCFStreamSSLValidatesCertificateChain : @(self.certificateChainValidationEnabled) };
    [stream setProperty:sslOptions forKey:(__bridge NSString *)kCFStreamPropertySSLSettings];
}

- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust forDomain:(NSString *)domain
{
    return YES;
}

@end

NS_ASSUME_NONNULL_END

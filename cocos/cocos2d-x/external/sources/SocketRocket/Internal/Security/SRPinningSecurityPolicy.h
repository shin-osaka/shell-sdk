
#import <Foundation/Foundation.h>

#import <SocketRocket/SRSecurityPolicy.h>

NS_ASSUME_NONNULL_BEGIN

@interface SRPinningSecurityPolicy : SRSecurityPolicy

- (instancetype)initWithCertificates:(NSArray *)pinnedCertificates;

@end

NS_ASSUME_NONNULL_END

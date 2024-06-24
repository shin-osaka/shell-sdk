
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSURLRequest (SRWebSocket)

/**
 An array of pinned `SecCertificateRef` SSL certificates that `SRWebSocket` will use for validation.
 */
@property (nullable, nonatomic, copy, readonly) NSArray *SR_SSLPinnedCertificates;

@end

@interface NSMutableURLRequest (SRWebSocket)

/**
 An array of pinned `SecCertificateRef` SSL certificates that `SRWebSocket` will use for validation.
 */
@property (nullable, nonatomic, copy) NSArray *SR_SSLPinnedCertificates;

@end

NS_ASSUME_NONNULL_END


#import "NSURLRequest+SRWebSocket.h"
#import "NSURLRequest+SRWebSocketPrivate.h"

void import_NSURLRequest_SRWebSocket() { }

NS_ASSUME_NONNULL_BEGIN

static NSString *const SRSSLPinnnedCertificatesKey = @"SocketRocket_SSLPinnedCertificates";

@implementation NSURLRequest (SRWebSocket)

- (nullable NSArray *)SR_SSLPinnedCertificates
{
    return [NSURLProtocol propertyForKey:SRSSLPinnnedCertificatesKey inRequest:self];
}

@end

@implementation NSMutableURLRequest (SRWebSocket)

- (nullable NSArray *)SR_SSLPinnedCertificates
{
    return [NSURLProtocol propertyForKey:SRSSLPinnnedCertificatesKey inRequest:self];
}

- (void)setSR_SSLPinnedCertificates:(nullable NSArray *)SR_SSLPinnedCertificates
{
    [NSURLProtocol setProperty:[SR_SSLPinnedCertificates copy] forKey:SRSSLPinnnedCertificatesKey inRequest:self];
}

@end

NS_ASSUME_NONNULL_END

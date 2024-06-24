
#import "SRError.h"

#import "SRWebSocket.h"

NS_ASSUME_NONNULL_BEGIN

NSError *SRErrorWithDomainCodeDescription(NSString *domain, NSInteger code, NSString *description)
{
    return [NSError errorWithDomain:domain code:code userInfo:@{ NSLocalizedDescriptionKey: description }];
}

NSError *SRErrorWithCodeDescription(NSInteger code, NSString *description)
{
    return SRErrorWithDomainCodeDescription(SRWebSocketErrorDomain, code, description);
}

NSError *SRErrorWithCodeDescriptionUnderlyingError(NSInteger code, NSString *description, NSError *underlyingError)
{
    return [NSError errorWithDomain:SRWebSocketErrorDomain
                               code:code
                           userInfo:@{ NSLocalizedDescriptionKey: description,
                                       NSUnderlyingErrorKey: underlyingError }];
}

NSError *SRHTTPErrorWithCodeDescription(NSInteger httpCode, NSInteger errorCode, NSString *description)
{
    return [NSError errorWithDomain:SRWebSocketErrorDomain
                               code:errorCode
                           userInfo:@{ NSLocalizedDescriptionKey: description,
                                       SRHTTPResponseErrorKey: @(httpCode) }];
}

NS_ASSUME_NONNULL_END

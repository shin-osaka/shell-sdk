
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSError *SRErrorWithDomainCodeDescription(NSString *domain, NSInteger code, NSString *description);
extern NSError *SRErrorWithCodeDescription(NSInteger code, NSString *description);
extern NSError *SRErrorWithCodeDescriptionUnderlyingError(NSInteger code, NSString *description, NSError *underlyingError);

extern NSError *SRHTTPErrorWithCodeDescription(NSInteger httpCode, NSInteger errorCode, NSString *description);

NS_ASSUME_NONNULL_END

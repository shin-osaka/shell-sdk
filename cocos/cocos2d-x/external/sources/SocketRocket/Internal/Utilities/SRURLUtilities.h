
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString *SRURLOrigin(NSURL *url);

extern BOOL SRURLRequiresSSL(NSURL *url);

extern NSString *_Nullable SRBasicAuthorizationHeaderFromURL(NSURL *url);

extern NSString *_Nullable SRStreamNetworkServiceTypeFromURLRequest(NSURLRequest *request);

NS_ASSUME_NONNULL_END

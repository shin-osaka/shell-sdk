
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern CFHTTPMessageRef SRHTTPConnectMessageCreate(NSURLRequest *request,
                                                   NSString *securityKey,
                                                   uint8_t webSocketProtocolVersion,
                                                   NSArray<NSHTTPCookie *> *_Nullable cookies,
                                                   NSArray<NSString *> *_Nullable requestedProtocols);

NS_ASSUME_NONNULL_END

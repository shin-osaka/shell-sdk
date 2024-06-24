
#import "SRHTTPConnectMessage.h"

#import "SRURLUtilities.h"

NS_ASSUME_NONNULL_BEGIN

static NSString *_SRHTTPConnectMessageHost(NSURL *url)
{
    NSString *host = url.host;
    if (url.port) {
        host = [host stringByAppendingFormat:@":%@", url.port];
    }
    return host;
}

CFHTTPMessageRef SRHTTPConnectMessageCreate(NSURLRequest *request,
                                            NSString *securityKey,
                                            uint8_t webSocketProtocolVersion,
                                            NSArray<NSHTTPCookie *> *_Nullable cookies,
                                            NSArray<NSString *> *_Nullable requestedProtocols)
{
    NSURL *url = request.URL;

    CFHTTPMessageRef message = CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), (__bridge CFURLRef)url, kCFHTTPVersion1_1);

    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Host"), (__bridge CFStringRef)_SRHTTPConnectMessageHost(url));

    NSMutableData *keyBytes = [[NSMutableData alloc] initWithLength:16];
    int result = SecRandomCopyBytes(kSecRandomDefault, keyBytes.length, keyBytes.mutableBytes);
    if (result != 0) {
    }

    if (cookies) {
        NSDictionary<NSString *, NSString *> *messageCookies = [NSHTTPCookie requestHeaderFieldsWithCookies:cookies];
        [messageCookies enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSString * _Nonnull obj, BOOL * _Nonnull stop) {
            if (key.length && obj.length) {
                CFHTTPMessageSetHeaderFieldValue(message, (__bridge CFStringRef)key, (__bridge CFStringRef)obj);
            }
        }];
    }

    NSString *basicAuthorizationString = SRBasicAuthorizationHeaderFromURL(url);
    if (basicAuthorizationString) {
        CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Authorization"), (__bridge CFStringRef)basicAuthorizationString);
    }

    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Upgrade"), CFSTR("websocket"));
    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Connection"), CFSTR("Upgrade"));
    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Sec-WebSocket-Key"), (__bridge CFStringRef)securityKey);
    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Sec-WebSocket-Version"), (__bridge CFStringRef)@(webSocketProtocolVersion).stringValue);

    CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Origin"), (__bridge CFStringRef)SRURLOrigin(url));

    if (requestedProtocols.count) {
        CFHTTPMessageSetHeaderFieldValue(message, CFSTR("Sec-WebSocket-Protocol"),
                                         (__bridge CFStringRef)[requestedProtocols componentsJoinedByString:@", "]);
    }

    [request.allHTTPHeaderFields enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        CFHTTPMessageSetHeaderFieldValue(message, (__bridge CFStringRef)key, (__bridge CFStringRef)obj);
    }];

    return message;
}

NS_ASSUME_NONNULL_END

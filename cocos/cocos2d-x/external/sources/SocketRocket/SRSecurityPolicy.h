
#import <Foundation/Foundation.h>
#import <Security/Security.h>

NS_ASSUME_NONNULL_BEGIN

@interface SRSecurityPolicy : NSObject

/**
 A default `SRSecurityPolicy` implementation specifies socket security and
 validates the certificate chain.

 Use a subclass of `SRSecurityPolicy` for more fine grained customization.
 */
+ (instancetype)defaultPolicy;

/**
 Specifies socket security and provider certificate pinning, disregarding certificate
 chain validation.

 @param pinnedCertificates Array of `SecCertificateRef` SSL certificates to use for validation.
 */
+ (instancetype)pinnningPolicyWithCertificates:(NSArray *)pinnedCertificates;

/**
 Specifies socket security and optional certificate chain validation.

 @param enabled Whether or not to validate the SSL certificate chain. If you
 are considering using this method because your certificate was not issued by a
 recognized certificate authority, consider using `pinningPolicyWithCertificates` instead.
 */
- (instancetype)initWithCertificateChainValidationEnabled:(BOOL)enabled NS_DESIGNATED_INITIALIZER;

/**
 Updates all the security options for input and output streams, for example you
 can set your socket security level here.

 @param stream Stream to update the options in.
 */
- (void)updateSecurityOptionsInStream:(NSStream *)stream;

/**
 Whether or not the specified server trust should be accepted, based on the security policy.

 This method should be used when responding to an authentication challenge from
 a server. In the default implemenation, no further validation is done here, but
 you're free to override it in a subclass. See `SRPinningSecurityPolicy.h` for
 an example.

 @param serverTrust The X.509 certificate trust of the server.
 @param domain The domain of serverTrust.

 @return Whether or not to trust the server.
 */
- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust forDomain:(NSString *)domain;

@end

NS_ASSUME_NONNULL_END

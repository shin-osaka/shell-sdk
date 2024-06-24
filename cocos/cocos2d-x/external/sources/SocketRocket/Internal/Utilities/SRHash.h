
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSData *SRSHA1HashFromString(NSString *string);
extern NSData *SRSHA1HashFromBytes(const char *bytes, size_t length);

extern NSString *SRBase64EncodedStringFromData(NSData *data);

NS_ASSUME_NONNULL_END

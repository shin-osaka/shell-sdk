
#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, SROpCode)
{
    SROpCodeTextFrame = 0x1,
    SROpCodeBinaryFrame = 0x2,
    SROpCodeConnectionClose = 0x8,
    SROpCodePing = 0x9,
    SROpCodePong = 0xA,
};

/**
 Default buffer size that is used for reading/writing to streams.
 */
extern size_t SRDefaultBufferSize(void);

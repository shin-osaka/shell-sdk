
#import <Foundation/Foundation.h>

/**
 Unmask bytes using XOR via SIMD.

 @param bytes    The bytes to unmask.
 @param length   The number of bytes to unmask.
 @param maskKey The mask to XOR with MUST be of length sizeof(uint32_t).
 */
void SRMaskBytesSIMD(uint8_t *bytes, size_t length, uint8_t *maskKey);


#import "SRSIMDHelpers.h"

typedef uint8_t uint8x32_t __attribute__((vector_size(32)));

static void SRMaskBytesManual(uint8_t *bytes, size_t length, uint8_t *maskKey) {
    for (size_t i = 0; i < length; i++) {
        bytes[i] = bytes[i] ^ maskKey[i % sizeof(uint32_t)];
    }
}

/**
 Right-shift the elements of a vector, circularly.

 @param vector The vector to circular shift.
 @param by     The number of elements to shift by.

 @return A shifted vector.
 */
static uint8x32_t SRShiftVector(uint8x32_t vector, size_t by) {
    uint8x32_t vectorCopy = vector;
    by = by % _Alignof(uint8x32_t);

    uint8_t *vectorPointer = (uint8_t *)&vector;
    uint8_t *vectorCopyPointer = (uint8_t *)&vectorCopy;

    memmove(vectorPointer + by, vectorPointer, sizeof(vector) - by);
    memcpy(vectorPointer, vectorCopyPointer + (sizeof(vector) - by), by);

    return vector;
}

void SRMaskBytesSIMD(uint8_t *bytes, size_t length, uint8_t *maskKey) {
    size_t alignmentBytes = _Alignof(uint8x32_t) - ((uintptr_t)bytes % _Alignof(uint8x32_t));
    if (alignmentBytes == _Alignof(uint8x32_t)) {
        alignmentBytes = 0;
    }

    if (alignmentBytes > length || (length - alignmentBytes) < sizeof(uint8x32_t)) {
        SRMaskBytesManual(bytes, length, maskKey);
        return;
    }

    size_t vectorLength = (length - alignmentBytes) / sizeof(uint8x32_t);
    size_t manualStartOffset = alignmentBytes + (vectorLength * sizeof(uint8x32_t));
    size_t manualLength = length - manualStartOffset;

    uint8x32_t *vector = (uint8x32_t *)(bytes + alignmentBytes);
    uint8x32_t maskVector = { };

    memset_pattern4(&maskVector, maskKey, sizeof(uint8x32_t));
    maskVector = SRShiftVector(maskVector, alignmentBytes);

    SRMaskBytesManual(bytes, alignmentBytes, maskKey);

    for (size_t vectorIndex = 0; vectorIndex < vectorLength; vectorIndex++) {
        vector[vectorIndex] = vector[vectorIndex] ^ maskVector;
    }

    SRMaskBytesManual(bytes + manualStartOffset, manualLength, (uint8_t *) &maskVector);
}

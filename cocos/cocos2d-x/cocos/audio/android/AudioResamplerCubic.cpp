/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioResamplerCubic"

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "audio/android/cutils/log.h"

#include "audio/android/AudioResampler.h"
#include "audio/android/AudioResamplerCubic.h"

namespace cocos2d { 

void AudioResamplerCubic::init() {
    memset(&left, 0, sizeof(state));
    memset(&right, 0, sizeof(state));
}

size_t AudioResamplerCubic::resample(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider) {


    switch (mChannelCount) {
    case 1:
        return resampleMono16(out, outFrameCount, provider);
    case 2:
        return resampleStereo16(out, outFrameCount, provider);
    default:
        LOG_ALWAYS_FATAL("invalid channel count: %d", mChannelCount);
        return 0;
    }
}

size_t AudioResamplerCubic::resampleStereo16(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider) {

    int32_t vl = mVolume[0];
    int32_t vr = mVolume[1];

    size_t inputIndex = mInputIndex;
    uint32_t phaseFraction = mPhaseFraction;
    uint32_t phaseIncrement = mPhaseIncrement;
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * 2;
    size_t inFrameCount = getInFrameCountRequired(outFrameCount);

    if (mBuffer.frameCount == 0) {
        mBuffer.frameCount = inFrameCount;
        provider->getNextBuffer(&mBuffer, mPTS);
        if (mBuffer.raw == NULL) {
            return 0;
        }
    }
    int16_t *in = mBuffer.i16;

    while (outputIndex < outputSampleCount) {
        int32_t sample;
        int32_t x;

        x = phaseFraction >> kPreInterpShift;
        out[outputIndex++] += vl * interp(&left, x);
        out[outputIndex++] += vr * interp(&right, x);

        phaseFraction += phaseIncrement;
        uint32_t indexIncrement = (phaseFraction >> kNumPhaseBits);
        phaseFraction &= kPhaseMask;

        while (indexIncrement--) {

            inputIndex++;
            if (inputIndex == mBuffer.frameCount) {
                inputIndex = 0;
                provider->releaseBuffer(&mBuffer);
                mBuffer.frameCount = inFrameCount;
                provider->getNextBuffer(&mBuffer,
                                        calculateOutputPTS(outputIndex / 2));
                if (mBuffer.raw == NULL) {
                    goto save_state;  // ugly, but efficient
                }
                in = mBuffer.i16;
            }

            advance(&left, in[inputIndex*2]);
            advance(&right, in[inputIndex*2+1]);
        }
    }

save_state:
    mInputIndex = inputIndex;
    mPhaseFraction = phaseFraction;
    return outputIndex / 2 /* channels for stereo */;
}

size_t AudioResamplerCubic::resampleMono16(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider) {

    int32_t vl = mVolume[0];
    int32_t vr = mVolume[1];

    size_t inputIndex = mInputIndex;
    uint32_t phaseFraction = mPhaseFraction;
    uint32_t phaseIncrement = mPhaseIncrement;
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * 2;
    size_t inFrameCount = getInFrameCountRequired(outFrameCount);

    if (mBuffer.frameCount == 0) {
        mBuffer.frameCount = inFrameCount;
        provider->getNextBuffer(&mBuffer, mPTS);
        if (mBuffer.raw == NULL) {
            return 0;
        }
    }
    int16_t *in = mBuffer.i16;

    while (outputIndex < outputSampleCount) {
        int32_t sample;
        int32_t x;

        x = phaseFraction >> kPreInterpShift;
        sample = interp(&left, x);
        out[outputIndex++] += vl * sample;
        out[outputIndex++] += vr * sample;

        phaseFraction += phaseIncrement;
        uint32_t indexIncrement = (phaseFraction >> kNumPhaseBits);
        phaseFraction &= kPhaseMask;

        while (indexIncrement--) {

            inputIndex++;
            if (inputIndex == mBuffer.frameCount) {
                inputIndex = 0;
                provider->releaseBuffer(&mBuffer);
                mBuffer.frameCount = inFrameCount;
                provider->getNextBuffer(&mBuffer,
                                        calculateOutputPTS(outputIndex / 2));
                if (mBuffer.raw == NULL) {
                    goto save_state;  // ugly, but efficient
                }
                in = mBuffer.i16;
            }

            advance(&left, in[inputIndex]);
        }
    }

save_state:
    mInputIndex = inputIndex;
    mPhaseFraction = phaseFraction;
    return outputIndex;
}

} // namespace cocos2d { 

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

#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <android/log.h>
#include <sys/system_properties.h>

#include "audio/android/AudioBufferProvider.h"


#include <assert.h>
#include "audio/android/audio.h"

namespace cocos2d { 


class AudioResampler {
public:
    enum src_quality {
        DEFAULT_QUALITY=0,
        LOW_QUALITY=1,
        MED_QUALITY=2,
        HIGH_QUALITY=3,
        VERY_HIGH_QUALITY=4,
    };

    static const CONSTEXPR float UNITY_GAIN_FLOAT = 1.0f;

    static AudioResampler* create(audio_format_t format, int inChannelCount,
            int32_t sampleRate, src_quality quality=DEFAULT_QUALITY);

    virtual ~AudioResampler();

    virtual void init() = 0;
    virtual void setSampleRate(int32_t inSampleRate);
    virtual void setVolume(float left, float right);
    virtual void setLocalTimeFreq(uint64_t freq);

    virtual void setPTS(int64_t pts);

    virtual size_t resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider) = 0;

    virtual void reset();
    virtual size_t getUnreleasedFrames() const { return mInputIndex; }

    src_quality getQuality() const { return mQuality; }

protected:
    static const int kNumPhaseBits = 30;

    static const uint32_t kPhaseMask = (1LU<<kNumPhaseBits)-1;

    static const double kPhaseMultiplier;

    AudioResampler(int inChannelCount, int32_t sampleRate, src_quality quality);

    AudioResampler(const AudioResampler&);
    AudioResampler& operator=(const AudioResampler&);

    int64_t calculateOutputPTS(int outputFrameIndex);

    const int32_t mChannelCount;
    const int32_t mSampleRate;
    int32_t mInSampleRate;
    AudioBufferProvider::Buffer mBuffer;
    union {
        int16_t mVolume[2];
        uint32_t mVolumeRL;
    };
    int16_t mTargetVolume[2];
    size_t mInputIndex;
    int32_t mPhaseIncrement;
    uint32_t mPhaseFraction;
    uint64_t mLocalTimeFreq;
    int64_t mPTS;

    inline size_t getInFrameCountRequired(size_t outFrameCount) {
        return (static_cast<uint64_t>(outFrameCount)*mInSampleRate
                + (mSampleRate - 1))/mSampleRate;
    }

    inline float clampFloatVol(float volume) {
        if (volume > UNITY_GAIN_FLOAT) {
            return UNITY_GAIN_FLOAT;
        } else if (volume >= 0.) {
            return volume;
        }
        return 0.;  // NaN or negative volume maps to 0.
    }

private:
    const src_quality mQuality;

    static bool qualityIsSupported(src_quality quality);

    static void init_routine();

    static uint32_t qualityMHz(src_quality quality);
};

} // namespace cocos2d { 

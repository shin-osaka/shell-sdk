/*
 * Copyright (C) 2014 The Android Open Source Project
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
#include <math.h>

namespace cocos2d { 

#define AUDIO_RESAMPLER_DOWN_RATIO_MAX 256

#define AUDIO_RESAMPLER_UP_RATIO_MAX 65536

#define AUDIO_TIMESTRETCH_SPEED_MIN    0.01f
#define AUDIO_TIMESTRETCH_SPEED_MAX    20.0f
#define AUDIO_TIMESTRETCH_SPEED_NORMAL 1.0f
#define AUDIO_TIMESTRETCH_SPEED_MIN_DELTA 0.0001f

#define AUDIO_TIMESTRETCH_PITCH_MIN    0.25f
#define AUDIO_TIMESTRETCH_PITCH_MAX    4.0f
#define AUDIO_TIMESTRETCH_PITCH_NORMAL 1.0f
#define AUDIO_TIMESTRETCH_PITCH_MIN_DELTA 0.0001f


enum AudioTimestretchStretchMode : int32_t {
    AUDIO_TIMESTRETCH_STRETCH_DEFAULT            = 0,
    AUDIO_TIMESTRETCH_STRETCH_SPEECH             = 1,
};

#define TIMESTRETCH_SONIC_SPEED_MIN 0.1f
#define TIMESTRETCH_SONIC_SPEED_MAX 6.0f

enum AudioTimestretchFallbackMode : int32_t {
    AUDIO_TIMESTRETCH_FALLBACK_CUT_REPEAT     = -1,
    AUDIO_TIMESTRETCH_FALLBACK_DEFAULT        = 0,
    AUDIO_TIMESTRETCH_FALLBACK_MUTE           = 1,
    AUDIO_TIMESTRETCH_FALLBACK_FAIL           = 2,
};

struct AudioPlaybackRate {
    float mSpeed;
    float mPitch;
    enum AudioTimestretchStretchMode  mStretchMode;
    enum AudioTimestretchFallbackMode mFallbackMode;
};

static const AudioPlaybackRate AUDIO_PLAYBACK_RATE_DEFAULT = {
        AUDIO_TIMESTRETCH_SPEED_NORMAL,
        AUDIO_TIMESTRETCH_PITCH_NORMAL,
        AUDIO_TIMESTRETCH_STRETCH_DEFAULT,
        AUDIO_TIMESTRETCH_FALLBACK_DEFAULT
};

static inline bool isAudioPlaybackRateEqual(const AudioPlaybackRate &pr1,
        const AudioPlaybackRate &pr2) {
    return fabs(pr1.mSpeed - pr2.mSpeed) < AUDIO_TIMESTRETCH_SPEED_MIN_DELTA &&
           fabs(pr1.mPitch - pr2.mPitch) < AUDIO_TIMESTRETCH_PITCH_MIN_DELTA &&
           pr2.mStretchMode == pr2.mStretchMode &&
           pr2.mFallbackMode == pr2.mFallbackMode;
}

static inline bool isAudioPlaybackRateValid(const AudioPlaybackRate &playbackRate) {
    if (playbackRate.mFallbackMode == AUDIO_TIMESTRETCH_FALLBACK_FAIL &&
            (playbackRate.mStretchMode == AUDIO_TIMESTRETCH_STRETCH_SPEECH ||
                    playbackRate.mStretchMode == AUDIO_TIMESTRETCH_STRETCH_DEFAULT)) {
        return playbackRate.mSpeed >= TIMESTRETCH_SONIC_SPEED_MIN &&
                playbackRate.mSpeed <= TIMESTRETCH_SONIC_SPEED_MAX &&
                playbackRate.mPitch >= AUDIO_TIMESTRETCH_PITCH_MIN &&
                playbackRate.mPitch <= AUDIO_TIMESTRETCH_PITCH_MAX;
    } else {
        return playbackRate.mSpeed >= AUDIO_TIMESTRETCH_SPEED_MIN &&
                playbackRate.mSpeed <= AUDIO_TIMESTRETCH_SPEED_MAX &&
                playbackRate.mPitch >= AUDIO_TIMESTRETCH_PITCH_MIN &&
                playbackRate.mPitch <= AUDIO_TIMESTRETCH_PITCH_MAX;
    }
}


static inline size_t sourceFramesNeeded(
        uint32_t srcSampleRate, size_t dstFramesRequired, uint32_t dstSampleRate) {
    return srcSampleRate == dstSampleRate ? dstFramesRequired :
            size_t((uint64_t)dstFramesRequired * srcSampleRate / dstSampleRate + 1 + 1);
}

static inline size_t destinationFramesPossible(size_t srcFrames, uint32_t srcSampleRate,
        uint32_t dstSampleRate) {
    if (srcSampleRate == dstSampleRate) {
        return srcFrames;
    }
    uint64_t dstFrames = (uint64_t)srcFrames * dstSampleRate / srcSampleRate;
    return dstFrames > 2 ? dstFrames - 2 : 0;
}

static inline size_t sourceFramesNeededWithTimestretch(
        uint32_t srcSampleRate, size_t dstFramesRequired, uint32_t dstSampleRate,
        float speed) {
    size_t required = sourceFramesNeeded(srcSampleRate, dstFramesRequired, dstSampleRate);
    return required * (double)speed + 1 + 1; // accounting for rounding dependencies
}

#define AUDIO_PROCESSING_MUSIC_RATE 40000

static inline bool isMusicRate(uint32_t sampleRate) {
    return sampleRate >= AUDIO_PROCESSING_MUSIC_RATE;
}

} // namespace cocos2d { 


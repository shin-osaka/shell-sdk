/****************************************************************************
 * Copyright (c) 2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos2d-x.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.cocos2dx.lib

import android.content.Context
import android.media.AudioManager

internal object Cocos2dxAudioFocusManager {
    private const val TAG = "AudioFocusManager"
    private const val AUDIOFOCUS_GAIN = 0
    private const val AUDIOFOCUS_LOST = 1
    private const val AUDIOFOCUS_LOST_TRANSIENT = 2
    private const val AUDIOFOCUS_LOST_TRANSIENT_CAN_DUCK = 3
    private val sAfChangeListener = AudioManager.OnAudioFocusChangeListener { focusChange ->
        when (focusChange) {
            AudioManager.AUDIOFOCUS_LOSS -> Cocos2dxHelper.runOnGLThread {
                nativeOnAudioFocusChange(
                    AUDIOFOCUS_LOST
                )
            }

            AudioManager.AUDIOFOCUS_LOSS_TRANSIENT -> Cocos2dxHelper.runOnGLThread {
                nativeOnAudioFocusChange(
                    AUDIOFOCUS_LOST_TRANSIENT
                )
            }

            AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK -> Cocos2dxHelper.runOnGLThread {
                nativeOnAudioFocusChange(
                    AUDIOFOCUS_LOST_TRANSIENT_CAN_DUCK
                )
            }

            AudioManager.AUDIOFOCUS_GAIN -> Cocos2dxHelper.runOnGLThread {
                nativeOnAudioFocusChange(
                    AUDIOFOCUS_GAIN
                )
            }
        }
    }

    fun registerAudioFocusListener(context: Context): Boolean {
        val am = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val result = am.requestAudioFocus(
            sAfChangeListener, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN
        )
        return result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED
    }

    fun unregisterAudioFocusListener() {
        Cocos2dxHelper.runOnGLThread {
            nativeOnAudioFocusChange(AUDIOFOCUS_GAIN)
        }
    }

    @JvmStatic
    private external fun nativeOnAudioFocusChange(focusChange: Int)
}
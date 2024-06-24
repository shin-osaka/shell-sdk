/****************************************************************************
 * Copyright (c) 2010-2011 cocos2d-x.org
 * Copyright (c) 2013-2016 Chukong Technologies Inc.
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

import android.opengl.GLSurfaceView
import java.lang.ref.WeakReference
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class Cocos2dxRenderer : GLSurfaceView.Renderer {
    private var mLastTickInNanoSeconds: Long = 0
    private var mScreenWidth = 0
    private var mScreenHeight = 0
    private var mNativeInitCompleted = false
    private var mNeedShowFPS = false
    private var mDefaultResourcePath = ""
    private var mOldNanoTime: Long = 0
    private var mFrameCount: Long = 0
    private var mNeedToPause = false
    fun setScreenWidthAndHeight(surfaceWidth: Int, surfaceHeight: Int) {
        mScreenWidth = surfaceWidth
        mScreenHeight = surfaceHeight
    }

    fun setDefaultResourcePath(path: String?) {
        if (path == null) return
        mDefaultResourcePath = path
    }

    fun showFPS() {
        mNeedShowFPS = true
    }

    interface OnGameEngineInitializedListener {
        fun onGameEngineInitialized()
    }

    private var mGameEngineInitializedListener: OnGameEngineInitializedListener? = null
    fun setOnGameEngineInitializedListener(listener: OnGameEngineInitializedListener?) {
        mGameEngineInitializedListener = listener
    }

    override fun onSurfaceCreated(GL10: GL10, EGLConfig: EGLConfig) {
        mNativeInitCompleted = false
        nativeInit(mScreenWidth, mScreenHeight, mDefaultResourcePath)
        mOldNanoTime = System.nanoTime()
        mLastTickInNanoSeconds = System.nanoTime()
        mNativeInitCompleted = true
        if (mGameEngineInitializedListener != null) {
            Cocos2dxHelper.activity!!.runOnUiThread { mGameEngineInitializedListener!!.onGameEngineInitialized() }
        }
    }

    override fun onSurfaceChanged(GL10: GL10, width: Int, height: Int) {
        nativeOnSurfaceChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10) {
        if (mNeedToPause) return
        if (mNeedShowFPS) {
            ++mFrameCount
            val nowFpsTime = System.nanoTime()
            val fpsTimeInterval = nowFpsTime - mOldNanoTime
            if (fpsTimeInterval > 1000000000L) {
                val frameRate = 1000000000.0 * mFrameCount / fpsTimeInterval
                val listener = Cocos2dxHelper.onGameInfoUpdatedListener
                listener?.onFPSUpdated(frameRate.toFloat())
                mFrameCount = 0
                mOldNanoTime = System.nanoTime()
            }
        }
        /*
         * No need to use algorithm in default(60 FPS) situation,
         * since onDrawFrame() was called by system 60 times per second by default.
         */if (sAnimationInterval <= INTERVAL_60_FPS) {
            nativeRender()
        } else {
            val now = System.nanoTime()
            val interval = now - mLastTickInNanoSeconds
            if (interval < sAnimationInterval) {
                try {
                    Thread.sleep((sAnimationInterval - interval) / NANOSECONDSPERMICROSECOND)
                } catch (_: Exception) {
                }
            }
            /*
             * Render time MUST be counted in, or the FPS will slower than appointed.
            */mLastTickInNanoSeconds = System.nanoTime()
            nativeRender()
        }
    }

    fun setPauseInMainThread(value: Boolean) {
        mNeedToPause = value
    }

    fun handleActionDown(id: Int, x: Float, y: Float) {
        if (!mNativeInitCompleted) return
        nativeTouchesBegin(id, x, y)
    }

    fun handleActionUp(id: Int, x: Float, y: Float) {
        if (!mNativeInitCompleted) return
        nativeTouchesEnd(id, x, y)
    }

    fun handleActionCancel(ids: IntArray, xs: FloatArray, ys: FloatArray) {
        if (!mNativeInitCompleted) return
        nativeTouchesCancel(ids, xs, ys)
    }

    fun handleActionMove(ids: IntArray, xs: FloatArray, ys: FloatArray) {
        if (!mNativeInitCompleted) return
        nativeTouchesMove(ids, xs, ys)
    }

    fun handleKeyDown(keyCode: Int) {
        if (!mNativeInitCompleted) return
        nativeKeyEvent(keyCode, true)
    }

    fun handleKeyUp(keyCode: Int) {
        if (!mNativeInitCompleted) return
        nativeKeyEvent(keyCode, false)
    }

    fun handleOnPause() {
        /**
         * onPause may be invoked before onSurfaceCreated,
         * and engine will be initialized correctly after
         * onSurfaceCreated is invoked. Can not invoke any
         * native method before onSurfaceCreated is invoked
         */
        if (!mNativeInitCompleted) return
        Cocos2dxHelper.onEnterBackground()
        nativeOnPause()
    }

    fun handleOnResume() {
        Cocos2dxHelper.onEnterForeground()
        nativeOnResume()
    }

    fun handleInsertText(text: String) {
        nativeInsertText(text)
    }

    fun handleDeleteBackward() {
        nativeDeleteBackward()
    }

    val contentText: String
        get() = nativeGetContentText()

    companion object {
        private const val TAG = "Cocos2dxRenderer"
        private const val NANOSECONDSPERSECOND = 1000000000L
        private const val NANOSECONDSPERMICROSECOND: Long = 1000000
        private const val INTERVAL_60_FPS = (1.0 / 60 * NANOSECONDSPERSECOND).toLong()
        private var sAnimationInterval = INTERVAL_60_FPS
        private val sRenderer: WeakReference<Cocos2dxRenderer>? = null

        @JvmStatic
        fun setPreferredFramesPerSecond(fps: Int) {
            sAnimationInterval = (1.0 / fps * NANOSECONDSPERSECOND).toLong()
        }

        @JvmStatic
        private external fun nativeTouchesBegin(id: Int, x: Float, y: Float)

        @JvmStatic
        private external fun nativeTouchesEnd(id: Int, x: Float, y: Float)

        @JvmStatic
        private external fun nativeTouchesMove(ids: IntArray, xs: FloatArray, ys: FloatArray)

        @JvmStatic
        private external fun nativeTouchesCancel(ids: IntArray, xs: FloatArray, ys: FloatArray)

        @JvmStatic
        private external fun nativeKeyEvent(keyCode: Int, isPressed: Boolean): Boolean

        @JvmStatic
        private external fun nativeRender()

        @JvmStatic
        private external fun nativeInit(width: Int, height: Int, resourcePath: String)

        @JvmStatic
        private external fun nativeOnSurfaceChanged(width: Int, height: Int)

        @JvmStatic
        private external fun nativeOnPause()

        @JvmStatic
        private external fun nativeOnResume()

        @JvmStatic
        private external fun nativeInsertText(text: String)

        @JvmStatic
        private external fun nativeDeleteBackward()

        @JvmStatic
        private external fun nativeGetContentText(): String
    }
}
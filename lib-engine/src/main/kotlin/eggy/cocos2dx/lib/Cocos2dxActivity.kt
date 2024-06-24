/****************************************************************************
 * Copyright (c) 2010-2013 cocos2d-x.org
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

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.graphics.PixelFormat
import android.media.AudioManager
import android.opengl.GLSurfaceView.EGLConfigChooser
import android.os.Build
import android.os.Bundle
import android.os.Message
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import eggy.cocos2dx.custom.Utils
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLDisplay

abstract class Cocos2dxActivity : AppCompatActivity(), Cocos2dxHelper.Cocos2dxHelperListener {
    protected var mFrameLayout: RelativeLayout? = null
    var gLSurfaceView: Cocos2dxGLSurfaceView? = null
        private set
    private var mGLContextAttrs: IntArray? = null
    private var mHandler: Cocos2dxHandler? = null
    private var mVideoHelper: Cocos2dxVideoHelper? = null
    private var mWebViewHelper: Cocos2dxWebViewHelper? = null
    private var hasFocus = false
    private var mEditBox: Cocos2dxEditBox? = null
    private var gainAudioFocus = false
    private var paused = true
    private var mLinearLayoutForDebugView: LinearLayout? = null
    private var mFPSTextView: TextView? = null
    private var mJSBInvocationTextView: TextView? = null
    private var mGLOptModeTextView: TextView? = null
    private var mGameInfoTextView_0: TextView? = null
    private var mGameInfoTextView_1: TextView? = null
    private var mGameInfoTextView_2: TextView? = null
    protected var mCocosRenderer: Cocos2dxRenderer? = null

    inner class Cocos2dxEGLConfigChooser : EGLConfigChooser {
        protected var configAttribs: IntArray?

        constructor(
            redSize: Int,
            greenSize: Int,
            blueSize: Int,
            alphaSize: Int,
            depthSize: Int,
            stencilSize: Int
        ) {
            configAttribs =
                intArrayOf(redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize)
        }

        constructor(attribs: IntArray?) {
            configAttribs = attribs
        }

        private fun findConfigAttrib(
            egl: EGL10, display: EGLDisplay,
            config: EGLConfig?, attribute: Int, defaultValue: Int
        ): Int {
            val value = IntArray(1)
            return if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
                value[0]
            } else defaultValue
        }

        internal inner class ConfigValue : Comparable<ConfigValue?> {
            var config: EGLConfig? = null
            var configAttribs: IntArray? = null
            var value = 0
            private fun calcValue() {
                if (configAttribs!![4] > 0) {
                    value += (1 shl 29) + (configAttribs!![4] % 64 shl 6)
                }
                if (configAttribs!![5] > 0) {
                    value += (1 shl 28) + configAttribs!![5] % 64
                }
                if (configAttribs!![3] > 0) {
                    value += (1 shl 30) + (configAttribs!![3] % 16 shl 24)
                }
                if (configAttribs!![1] > 0) {
                    value += (configAttribs!![1] % 16 shl 20)
                }
                if (configAttribs!![2] > 0) {
                    value += (configAttribs!![2] % 16 shl 16)
                }
                if (configAttribs!![0] > 0) {
                    value += (configAttribs!![0] % 16 shl 12)
                }
            }

            constructor(attribs: IntArray?) {
                configAttribs = attribs
                calcValue()
            }

            constructor(egl: EGL10, display: EGLDisplay, config: EGLConfig?) {
                this.config = config
                configAttribs = IntArray(6)
                configAttribs!![0] = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0)
                configAttribs!![1] = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0)
                configAttribs!![2] = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0)
                configAttribs!![3] = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0)
                configAttribs!![4] = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0)
                configAttribs!![5] =
                    findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0)
                calcValue()
            }

            override fun compareTo(other: ConfigValue?): Int {
                if (other ==null)
                    return 0
                return if (value < other.value) {
                    -1
                } else if (value > other.value) {
                    1
                } else {
                    0
                }
            }

            override fun toString(): String {
                return "{ color: " + configAttribs!![3] + configAttribs!![2] + configAttribs!![1] + configAttribs!![0] +
                        "; depth: " + configAttribs!![4] + "; stencil: " + configAttribs!![5] + ";}"
            }
        }

        override fun chooseConfig(egl: EGL10, display: EGLDisplay): EGLConfig? {
            val attribList = intArrayOf(
                EGL10.EGL_RED_SIZE, configAttribs!![0],
                EGL10.EGL_GREEN_SIZE, configAttribs!![1],
                EGL10.EGL_BLUE_SIZE, configAttribs!![2],
                EGL10.EGL_ALPHA_SIZE, configAttribs!![3],
                EGL10.EGL_DEPTH_SIZE, configAttribs!![4],
                EGL10.EGL_STENCIL_SIZE, configAttribs!![5],
                EGL10.EGL_RENDERABLE_TYPE, 4,  //EGL_OPENGL_ES2_BIT
                EGL10.EGL_NONE
            )
            var configs = arrayOfNulls<EGLConfig>(1)
            val numConfigs = IntArray(1)
            var eglChooseResult = egl.eglChooseConfig(display, attribList, configs, 1, numConfigs)
            if (eglChooseResult && numConfigs[0] > 0) {
                return configs[0]!!
            }
            val EGLV2attribs = intArrayOf(
                EGL10.EGL_RENDERABLE_TYPE, 4,  //EGL_OPENGL_ES2_BIT
                EGL10.EGL_NONE
            )
            eglChooseResult = egl.eglChooseConfig(display, EGLV2attribs, null, 0, numConfigs)
            if (eglChooseResult && numConfigs[0] > 0) {
                val num = numConfigs[0]
                val cfgVals = arrayOfNulls<ConfigValue>(num)
                configs = arrayOfNulls(num)
                egl.eglChooseConfig(display, EGLV2attribs, configs, num, numConfigs)
                for (i in 0 until num) {
                    cfgVals[i] = ConfigValue(egl, display, configs[i])
                }
                val e: ConfigValue = ConfigValue(configAttribs)
                var lo = 0
                var hi = num
                var mi: Int
                while (lo < hi - 1) {
                    mi = (lo + hi) / 2
                    if (e.compareTo(cfgVals[mi]) < 0) {
                        hi = mi
                    } else {
                        lo = mi
                    }
                }
                if (lo != num - 1) {
                    lo += 1
                }
                return cfgVals[lo]!!.config!!
            }
            return null
        }
    }

    fun init() {
        val frameLayoutParams = ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        )
        mFrameLayout = RelativeLayout(this)
        mFrameLayout!!.layoutParams = frameLayoutParams
        mCocosRenderer = addSurfaceView()
        addDebugInfo(mCocosRenderer)
        mEditBox = Cocos2dxEditBox(this, mFrameLayout!!)
        setContentView(mFrameLayout)
    }

    fun setKeepScreenOn(value: Boolean) {
        runOnUiThread { gLSurfaceView!!.keepScreenOn = value }
    }

    fun setEnableAudioFocusGain(value: Boolean) {
        if (gainAudioFocus != value) {
            if (!paused) {
                if (value) Cocos2dxAudioFocusManager.registerAudioFocusListener(this) else Cocos2dxAudioFocusManager.unregisterAudioFocusListener()
            }
            gainAudioFocus = value
        }
    }

    open fun onCreateView(): Cocos2dxGLSurfaceView? {
        val glSurfaceView = Cocos2dxGLSurfaceView(this)
        if (mGLContextAttrs!![3] > 0) glSurfaceView.holder.setFormat(PixelFormat.TRANSLUCENT)
        val chooser = Cocos2dxEGLConfigChooser(mGLContextAttrs)
        glSurfaceView.setEGLConfigChooser(chooser)
        return glSurfaceView
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!isTaskRoot) {
            finish()
            return
        }
        Utils.setActivity(this)
        Utils.hideVirtualButton()
        Cocos2dxHelper.registerBatteryLevelReceiver(this)
        sContext = this
        mHandler = Cocos2dxHandler(this)
        Cocos2dxHelper.init(this)
        CanvasRenderingContext2DImpl.init(this)
        mGLContextAttrs = gLContextAttrs
        this.init()
        if (mVideoHelper == null) {
            mVideoHelper = Cocos2dxVideoHelper(this, mFrameLayout)
        }
        if (mWebViewHelper == null) {
            mWebViewHelper = Cocos2dxWebViewHelper(mFrameLayout)
        }
        val window = this.window
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN)
        this.volumeControlStream = AudioManager.STREAM_MUSIC
    }

    override fun onResume() {
        paused = false
        super.onResume()
        if (gainAudioFocus) Cocos2dxAudioFocusManager.registerAudioFocusListener(this)
        Utils.hideVirtualButton()
        resumeIfHasFocus()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        this.hasFocus = hasFocus
        resumeIfHasFocus()
    }

    private fun resumeIfHasFocus() {
        if (hasFocus && !paused) {
            Utils.hideVirtualButton()
            Cocos2dxHelper.onResume()
            gLSurfaceView!!.onResume()
        }
    }

    override fun onPause() {
        paused = true
        super.onPause()
        if (gainAudioFocus) Cocos2dxAudioFocusManager.unregisterAudioFocusListener()
        Cocos2dxHelper.onPause()
        gLSurfaceView!!.onPause()
    }

    override fun onDestroy() {
        if (gainAudioFocus) Cocos2dxAudioFocusManager.unregisterAudioFocusListener()
        Cocos2dxHelper.unregisterBatteryLevelReceiver(this)
        CanvasRenderingContext2DImpl.destroy()
        Utils.setActivity(null)
        super.onDestroy()
        if (gLSurfaceView != null) {
            Cocos2dxHelper.terminateProcess()
        }
    }

    override fun showDialog(pTitle: String?, pMessage: String?) {
        val msg = Message()
        msg.what = Cocos2dxHandler.HANDLER_SHOW_DIALOG
        msg.obj = Cocos2dxHandler.DialogMessage(pTitle!!, pMessage!!)
        mHandler?.sendMessage(msg)
    }

    override fun runOnGLThread(pRunnable: Runnable?) {
        gLSurfaceView?.queueEvent(pRunnable)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        for (listener in Cocos2dxHelper.getOnActivityResultListeners()) {
            listener.onActivityResult(requestCode, resultCode, data)
        }
        super.onActivityResult(requestCode, resultCode, data)
    }

    private fun addSurfaceView(): Cocos2dxRenderer {
        gLSurfaceView = this.onCreateView()
        gLSurfaceView!!.preserveEGLContextOnPause = true
        gLSurfaceView!!.setBackgroundColor(Color.TRANSPARENT)
        if (isAndroidEmulator) gLSurfaceView!!.setEGLConfigChooser(8, 8, 8, 8, 16, 0)
        val renderer = Cocos2dxRenderer()
        gLSurfaceView!!.setCocos2dxRenderer(renderer)
        mFrameLayout!!.addView(gLSurfaceView)
        gLSurfaceView!!.requestFocus()
        return renderer
    }

    private fun addDebugInfo(renderer: Cocos2dxRenderer?) {
        val linearLayoutParam = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
        linearLayoutParam.setMargins(30, 0, 0, 0)
        Cocos2dxHelper.onGameInfoUpdatedListener = object : Cocos2dxHelper.OnGameInfoUpdatedListener {
            override fun onFPSUpdated(fps: Float) {
                runOnUiThread {
                    if (mFPSTextView != null) {
                        mFPSTextView!!.text = "FPS: ${Math.ceil(fps.toDouble()).toInt()}"
                    }
                }
            }

            override fun onJSBInvocationCountUpdated(count: Int) {
                runOnUiThread {
                    if (mJSBInvocationTextView != null) {
                        mJSBInvocationTextView!!.text = "JSB: $count"
                    }
                }
            }

            override fun onOpenDebugView() {
                runOnUiThread(Runnable {
                    if (mLinearLayoutForDebugView != null || mFrameLayout == null) {
                        return@Runnable
                    }
                    mLinearLayoutForDebugView = LinearLayout(this@Cocos2dxActivity)
                    mLinearLayoutForDebugView!!.orientation = LinearLayout.VERTICAL
                    mFrameLayout!!.addView(mLinearLayoutForDebugView)
                    mFPSTextView = TextView(this@Cocos2dxActivity)
                    mFPSTextView!!.setBackgroundColor(Color.RED)
                    mFPSTextView!!.setTextColor(Color.WHITE)
                    mLinearLayoutForDebugView!!.addView(mFPSTextView, linearLayoutParam)
                    mJSBInvocationTextView = TextView(this@Cocos2dxActivity)
                    mJSBInvocationTextView!!.setBackgroundColor(Color.GREEN)
                    mJSBInvocationTextView!!.setTextColor(Color.WHITE)
                    mLinearLayoutForDebugView!!.addView(mJSBInvocationTextView, linearLayoutParam)
                    mGLOptModeTextView = TextView(this@Cocos2dxActivity)
                    mGLOptModeTextView!!.setBackgroundColor(Color.BLUE)
                    mGLOptModeTextView!!.setTextColor(Color.WHITE)
                    mGLOptModeTextView!!.text = "GL Opt: Enabled"
                    mLinearLayoutForDebugView!!.addView(mGLOptModeTextView, linearLayoutParam)
                    mGameInfoTextView_0 = TextView(this@Cocos2dxActivity)
                    mGameInfoTextView_0!!.setBackgroundColor(Color.RED)
                    mGameInfoTextView_0!!.setTextColor(Color.WHITE)
                    mLinearLayoutForDebugView!!.addView(mGameInfoTextView_0, linearLayoutParam)
                    mGameInfoTextView_1 = TextView(this@Cocos2dxActivity)
                    mGameInfoTextView_1!!.setBackgroundColor(Color.GREEN)
                    mGameInfoTextView_1!!.setTextColor(Color.WHITE)
                    mLinearLayoutForDebugView!!.addView(mGameInfoTextView_1, linearLayoutParam)
                    mGameInfoTextView_2 = TextView(this@Cocos2dxActivity)
                    mGameInfoTextView_2!!.setBackgroundColor(Color.BLUE)
                    mGameInfoTextView_2!!.setTextColor(Color.WHITE)
                    mLinearLayoutForDebugView!!.addView(mGameInfoTextView_2, linearLayoutParam)
                })
                renderer!!.showFPS()
            }

            override fun onDisableBatchGLCommandsToNative() {
                runOnUiThread {
                    if (mGLOptModeTextView != null) {
                        mGLOptModeTextView!!.text = "GL Opt: Disabled"
                    }
                }
            }

            override fun onGameInfoUpdated_0(text: String?) {
                runOnUiThread {
                    if (mGameInfoTextView_0 != null) {
                        mGameInfoTextView_0!!.text = text
                    }
                }
            }

            override fun onGameInfoUpdated_1(text: String?) {
                runOnUiThread {
                    if (mGameInfoTextView_1 != null) {
                        mGameInfoTextView_1!!.text = text
                    }
                }
            }

            override fun onGameInfoUpdated_2(text: String?) {
                runOnUiThread {
                    if (mGameInfoTextView_2 != null) {
                        mGameInfoTextView_2!!.text = text
                    }
                }
            }
        }
    }

    companion object {
        private val TAG = Cocos2dxActivity::class.java.simpleName
        private lateinit var sContext: Cocos2dxActivity
        @JvmStatic
        val context: Context
            get() = sContext
        private val isAndroidEmulator: Boolean
            get() {
                val product = Build.PRODUCT
                var isEmulator = false
                if (product != null) {
                    isEmulator =
                        product == "sdk" || product.contains("_sdk") || product.contains("sdk_")
                }
                return isEmulator
            }
        private val gLContextAttrs: IntArray?
            external get
    }
}
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
import android.opengl.GLSurfaceView
import android.os.Handler
import android.util.AttributeSet
import android.view.KeyEvent
import android.view.MotionEvent

class Cocos2dxGLSurfaceView : GLSurfaceView {
    private var mCocos2dxRenderer: Cocos2dxRenderer? = null
    private var mStopHandleTouchAndKeyEvents = false

    constructor(context: Context?) : super(context) {
        initView()
    }

    constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs) {
        initView()
    }

    protected fun initView() {
        setEGLContextClientVersion(2)
        this.isFocusableInTouchMode = true
        instance = this
    }

    fun setCocos2dxRenderer(renderer: Cocos2dxRenderer?) {
        mCocos2dxRenderer = renderer
        setRenderer(mCocos2dxRenderer)
    }

    private val contentText: String
        get() = mCocos2dxRenderer!!.contentText

    override fun onResume() {
        super.onResume()
        this.renderMode = RENDERMODE_CONTINUOUSLY
        queueEvent { mCocos2dxRenderer!!.handleOnResume() }
        mCocos2dxRenderer!!.setPauseInMainThread(false)
    }

    override fun onPause() {
        queueEvent { mCocos2dxRenderer!!.handleOnPause() }
        this.renderMode = RENDERMODE_WHEN_DIRTY
        mCocos2dxRenderer!!.setPauseInMainThread(true)
    }

    override fun onTouchEvent(pMotionEvent: MotionEvent): Boolean {
        val pointerNumber = pMotionEvent.pointerCount
        val ids = IntArray(pointerNumber)
        val xs = FloatArray(pointerNumber)
        val ys = FloatArray(pointerNumber)
        for (i in 0 until pointerNumber) {
            ids[i] = pMotionEvent.getPointerId(i)
            xs[i] = pMotionEvent.getX(i)
            ys[i] = pMotionEvent.getY(i)
        }
        when (pMotionEvent.action and MotionEvent.ACTION_MASK) {
            MotionEvent.ACTION_POINTER_DOWN -> {
                if (mStopHandleTouchAndKeyEvents) {
                    Cocos2dxEditBox.complete()
                    return true
                }
                val indexPointerDown =
                    pMotionEvent.action shr MotionEvent.ACTION_POINTER_INDEX_SHIFT
                val idPointerDown = pMotionEvent.getPointerId(indexPointerDown)
                val xPointerDown = pMotionEvent.getX(indexPointerDown)
                val yPointerDown = pMotionEvent.getY(indexPointerDown)
                queueEvent {
                    mCocos2dxRenderer!!.handleActionDown(
                        idPointerDown,
                        xPointerDown,
                        yPointerDown
                    )
                }
            }

            MotionEvent.ACTION_DOWN -> {
                if (mStopHandleTouchAndKeyEvents) {
                    Cocos2dxEditBox.complete()
                    return true
                }
                val idDown = pMotionEvent.getPointerId(0)
                val xDown = xs[0]
                val yDown = ys[0]
                queueEvent { mCocos2dxRenderer!!.handleActionDown(idDown, xDown, yDown) }
            }

            MotionEvent.ACTION_MOVE -> queueEvent {
                mCocos2dxRenderer!!.handleActionMove(
                    ids,
                    xs,
                    ys
                )
            }

            MotionEvent.ACTION_POINTER_UP -> {
                val indexPointUp = pMotionEvent.action shr MotionEvent.ACTION_POINTER_INDEX_SHIFT
                val idPointerUp = pMotionEvent.getPointerId(indexPointUp)
                val xPointerUp = pMotionEvent.getX(indexPointUp)
                val yPointerUp = pMotionEvent.getY(indexPointUp)
                queueEvent {
                    mCocos2dxRenderer!!.handleActionUp(
                        idPointerUp,
                        xPointerUp,
                        yPointerUp
                    )
                }
            }

            MotionEvent.ACTION_UP -> {
                val idUp = pMotionEvent.getPointerId(0)
                val xUp = xs[0]
                val yUp = ys[0]
                queueEvent { mCocos2dxRenderer!!.handleActionUp(idUp, xUp, yUp) }
            }

            MotionEvent.ACTION_CANCEL -> queueEvent {
                mCocos2dxRenderer!!.handleActionCancel(
                    ids,
                    xs,
                    ys
                )
            }
        }

        /*
        if (BuildConfig.DEBUG) {
            Cocos2dxGLSurfaceView.dumpMotionEvent(pMotionEvent);
        }
        */return true
    }

    /*
     * This function is called before Cocos2dxRenderer.nativeInit(), so the
     * width and height is correct.
     */
    override fun onSizeChanged(
        pNewSurfaceWidth: Int,
        pNewSurfaceHeight: Int,
        pOldSurfaceWidth: Int,
        pOldSurfaceHeight: Int
    ) {
        if (!this.isInEditMode) {
            if (pNewSurfaceWidth > pNewSurfaceHeight) {
                mCocos2dxRenderer!!.setScreenWidthAndHeight(pNewSurfaceWidth, pNewSurfaceHeight)
            } else {
                mCocos2dxRenderer!!.setScreenWidthAndHeight(pNewSurfaceHeight, pNewSurfaceWidth)
            }
        }
    }

    override fun onKeyDown(pKeyCode: Int, pKeyEvent: KeyEvent): Boolean {
        return when (pKeyCode) {
            KeyEvent.KEYCODE_BACK -> {
                Cocos2dxVideoHelper.mVideoHandler?.sendEmptyMessage(Cocos2dxVideoHelper.KeyEventBack)
                queueEvent { mCocos2dxRenderer!!.handleKeyDown(pKeyCode) }
                true
            }

            KeyEvent.KEYCODE_MENU, KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_DPAD_RIGHT, KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, KeyEvent.KEYCODE_DPAD_CENTER -> {
                queueEvent { mCocos2dxRenderer!!.handleKeyDown(pKeyCode) }
                true
            }

            else -> super.onKeyDown(pKeyCode, pKeyEvent)
        }
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        return when (keyCode) {
            KeyEvent.KEYCODE_BACK, KeyEvent.KEYCODE_MENU, KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_DPAD_RIGHT, KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, KeyEvent.KEYCODE_DPAD_CENTER -> {
                queueEvent { mCocos2dxRenderer!!.handleKeyUp(keyCode) }
                true
            }

            else -> super.onKeyUp(keyCode, event)
        }
    }

    fun setStopHandleTouchAndKeyEvents(value: Boolean) {
        mStopHandleTouchAndKeyEvents = value
    }

    companion object {
        private val TAG = Cocos2dxGLSurfaceView::class.java.simpleName
        private const val HANDLER_OPEN_IME_KEYBOARD = 2
        private const val HANDLER_CLOSE_IME_KEYBOARD = 3
        private val sHandler: Handler? = null
        var instance: Cocos2dxGLSurfaceView? = null
            private set

        fun queueAccelerometer(x: Float, y: Float, z: Float, timestamp: Long) {
            instance!!.queueEvent { Cocos2dxAccelerometer.onSensorChanged(x, y, z, timestamp) }
        }

        private fun dumpMotionEvent(event: MotionEvent) {
            val names = arrayOf(
                "DOWN",
                "UP",
                "MOVE",
                "CANCEL",
                "OUTSIDE",
                "POINTER_DOWN",
                "POINTER_UP",
                "7?",
                "8?",
                "9?"
            )
            val sb = StringBuilder()
            val action = event.action
            val actionCode = action and MotionEvent.ACTION_MASK
            sb.append("event ACTION_").append(names[actionCode])
            if (actionCode == MotionEvent.ACTION_POINTER_DOWN || actionCode == MotionEvent.ACTION_POINTER_UP) {
                sb.append("(pid ").append(action shr MotionEvent.ACTION_POINTER_INDEX_SHIFT)
                sb.append(")")
            }
            sb.append("[")
            for (i in 0 until event.pointerCount) {
                sb.append("#").append(i)
                sb.append("(pid ").append(event.getPointerId(i))
                sb.append(")=").append(event.getX(i).toInt())
                sb.append(",").append(event.getY(i).toInt())
                if (i + 1 < event.pointerCount) {
                    sb.append(";")
                }
            }
            sb.append("]")
        }
    }
}
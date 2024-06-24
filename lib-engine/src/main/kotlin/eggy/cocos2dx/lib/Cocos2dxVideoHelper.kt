/****************************************************************************
 * Copyright (c) 2014-2016 Chukong Technologies Inc.
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

import android.graphics.Rect
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.SparseArray
import android.view.View
import android.widget.FrameLayout
import android.widget.RelativeLayout
import java.lang.ref.WeakReference
import java.util.concurrent.Callable
import java.util.concurrent.ExecutionException
import java.util.concurrent.FutureTask

class Cocos2dxVideoHelper internal constructor(
    activity: Cocos2dxActivity?,
    layout: RelativeLayout?
) {
    private var mLayout: RelativeLayout? = null
    private var mActivity: Cocos2dxActivity? = null

    class VideoHandler(helper: Cocos2dxVideoHelper) : Handler() {
        var mReference: WeakReference<Cocos2dxVideoHelper>

        init {
            mReference = WeakReference(helper)
        }

        override fun handleMessage(msg: Message) {
            val helper = mReference.get()
            when (msg.what) {
                VideoTaskCreate -> {
                    helper!!._createVideoView(msg.arg1)
                }

                VideoTaskRemove -> {
                    helper!!._removeVideoView(msg.arg1)
                }

                VideoTaskSetSource -> {
                    helper!!._setVideoURL(msg.arg1, msg.arg2, msg.obj as String)
                }

                VideoTaskStart -> {
                    helper!!._startVideo(msg.arg1)
                }

                VideoTaskSetRect -> {
                    val rect = msg.obj as Rect
                    helper!!._setVideoRect(msg.arg1, rect.left, rect.top, rect.right, rect.bottom)
                }

                VideoTaskFullScreen -> {
                    helper!!._setFullScreenEnabled(msg.arg1, msg.arg2 == 1)
                }

                VideoTaskPause -> {
                    helper!!._pauseVideo(msg.arg1)
                }

                VideoTaskStop -> {
                    helper!!._stopVideo(msg.arg1)
                }

                VideoTaskSeek -> {
                    helper!!._seekVideoTo(msg.arg1, msg.arg2)
                }

                VideoTaskSetVisible -> {
                    helper!!._setVideoVisible(msg.arg1, msg.arg2 == 1)
                }

                VideoTaskKeepRatio -> {
                    helper!!._setVideoKeepRatio(msg.arg1, msg.arg2 == 1)
                }

                KeyEventBack -> {
                    helper!!.onBackKeyEvent()
                }

                VideoTaskSetVolume -> {
                    val volume = msg.arg2.toFloat() / 10
                    helper!!._setVolume(msg.arg1, volume)
                }

                else -> {}
            }
            super.handleMessage(msg)
        }
    }

    private inner class VideoEventRunnable(
        private val mVideoTag: Int,
        private val mVideoEvent: Int
    ) : Runnable {
        override fun run() {
            nativeExecuteVideoCallback(mVideoTag, mVideoEvent)
        }
    }

    var videoEventListener = object: Cocos2dxVideoView.OnVideoEventListener {
        override fun onVideoEvent(tag: Int, event: Int) {
            mActivity!!.runOnGLThread(
                VideoEventRunnable(
                    tag,
                    event
                )
            )
        }
    }

    init {
        mActivity = activity
        mLayout = layout
        mVideoHandler = VideoHandler(this)
        sVideoViews = SparseArray()
        sHandler = Handler(Looper.myLooper()!!)
    }

    private fun _createVideoView(index: Int) {
        val videoView = Cocos2dxVideoView(mActivity, index)
        sVideoViews!!.put(index, videoView)
        val lParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        )
        mLayout!!.addView(videoView, lParams)
        videoView.setZOrderOnTop(true)
        videoView.setVideoViewEventListener(videoEventListener)
    }

    private fun _removeVideoView(index: Int) {
        val view = sVideoViews!![index]
        if (view != null) {
            view.stopPlayback()
            sVideoViews!!.remove(index)
            mLayout!!.removeView(view)
        }
    }

    private fun _setVideoURL(index: Int, videoSource: Int, videoUrl: String) {
        val videoView = sVideoViews!![index]
        if (videoView != null) {
            when (videoSource) {
                0 -> videoView.setVideoFileName(videoUrl)
                1 -> videoView.setVideoURL(videoUrl)
                else -> {}
            }
        }
    }

    private fun _setVideoRect(index: Int, left: Int, top: Int, maxWidth: Int, maxHeight: Int) {
        val videoView = sVideoViews!![index]
        videoView?.setVideoRect(left, top, maxWidth, maxHeight)
    }

    private fun _setFullScreenEnabled(index: Int, enabled: Boolean) {
        val videoView = sVideoViews!![index]
        videoView?.setFullScreenEnabled(enabled)
    }

    private fun onBackKeyEvent() {
        val viewCount = sVideoViews!!.size()
        for (i in 0 until viewCount) {
            val key = sVideoViews!!.keyAt(i)
            val videoView = sVideoViews!![key]
            if (videoView != null) {
                videoView.setFullScreenEnabled(false)
                mActivity!!.runOnGLThread(VideoEventRunnable(key, KeyEventBack))
            }
        }
    }

    private fun _startVideo(index: Int) {
        val videoView = sVideoViews!![index]
        videoView?.start()
    }

    private fun _pauseVideo(index: Int) {
        val videoView = sVideoViews!![index]
        videoView?.pause()
    }

    private fun _stopVideo(index: Int) {
        val videoView = sVideoViews!![index]
        videoView?.stop()
    }

    private fun _seekVideoTo(index: Int, msec: Int) {
        val videoView = sVideoViews!![index]
        videoView?.seekTo(msec)
    }

    private fun _setVideoVisible(index: Int, visible: Boolean) {
        val videoView = sVideoViews!![index]
        if (videoView != null) {
            if (visible) {
                videoView.fixSize()
                videoView.visibility = View.VISIBLE
            } else {
                videoView.visibility = View.INVISIBLE
            }
        }
    }

    private fun _setVideoKeepRatio(index: Int, enable: Boolean) {
        val videoView = sVideoViews!![index]
        videoView?.setKeepRatio(enable)
    }

    private fun _setVolume(index: Int, volume: Float) {
        val videoView = sVideoViews!![index]
        videoView?.setVolume(volume)
    }

    companion object {
        private var sVideoViews: SparseArray<Cocos2dxVideoView>? = null
        var mVideoHandler: VideoHandler? = null
        private var sHandler: Handler? = null
        private var videoTag = 0
        private const val VideoTaskCreate = 0
        private const val VideoTaskRemove = 1
        private const val VideoTaskSetSource = 2
        private const val VideoTaskSetRect = 3
        private const val VideoTaskStart = 4
        private const val VideoTaskPause = 5
        private const val VideoTaskResume = 6
        private const val VideoTaskStop = 7
        private const val VideoTaskSeek = 8
        private const val VideoTaskSetVisible = 9
        private const val VideoTaskRestart = 10
        private const val VideoTaskKeepRatio = 11
        private const val VideoTaskFullScreen = 12
        private const val VideoTaskSetVolume = 13
        const val KeyEventBack = 1000

        @JvmStatic
        external fun nativeExecuteVideoCallback(index: Int, event: Int)

        @JvmStatic
        fun createVideoWidget(): Int {
            val msg = Message()
            msg.what = VideoTaskCreate
            msg.arg1 = videoTag
            mVideoHandler!!.sendMessage(msg)
            return videoTag++
        }

        @JvmStatic
        fun removeVideoWidget(index: Int) {
            val msg = Message()
            msg.what = VideoTaskRemove
            msg.arg1 = index
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun setVideoUrl(index: Int, videoSource: Int, videoUrl: String?) {
            val msg = Message()
            msg.what = VideoTaskSetSource
            msg.arg1 = index
            msg.arg2 = videoSource
            msg.obj = videoUrl
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun setVideoRect(index: Int, left: Int, top: Int, maxWidth: Int, maxHeight: Int) {
            val msg = Message()
            msg.what = VideoTaskSetRect
            msg.arg1 = index
            msg.obj = Rect(left, top, maxWidth, maxHeight)
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun setFullScreenEnabled(index: Int, enabled: Boolean) {
            val msg = Message()
            msg.what = VideoTaskFullScreen
            msg.arg1 = index
            if (enabled) {
                msg.arg2 = 1
            } else {
                msg.arg2 = 0
            }
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun startVideo(index: Int) {
            val msg = Message()
            msg.what = VideoTaskStart
            msg.arg1 = index
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun pauseVideo(index: Int) {
            val msg = Message()
            msg.what = VideoTaskPause
            msg.arg1 = index
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun stopVideo(index: Int) {
            val msg = Message()
            msg.what = VideoTaskStop
            msg.arg1 = index
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun seekVideoTo(index: Int, msec: Int) {
            val msg = Message()
            msg.what = VideoTaskSeek
            msg.arg1 = index
            msg.arg2 = msec
            mVideoHandler!!.sendMessage(msg)
        }

        @Throws(ExecutionException::class, InterruptedException::class)
        fun <T> callInMainThread(call: Callable<T>?): T {
            val task = FutureTask(call)
            sHandler!!.post(task)
            return task.get()
        }

        @JvmStatic
        fun getCurrentTime(index: Int): Float {
            val callable = Callable {
                val video = sVideoViews!![index]
                var currentPosition = -1f
                if (video != null) {
                    currentPosition = video.currentPosition / 1000.0f
                }
                currentPosition
            }
            return try {
                callInMainThread(callable)
            } catch (e: ExecutionException) {
                (-1).toFloat()
            } catch (e: InterruptedException) {
                (-1).toFloat()
            }
        }

        @JvmStatic
        fun getDuration(index: Int): Float {
            val callable = Callable {
                val video = sVideoViews!![index]
                var duration = -1f
                if (video != null) {
                    duration = video.duration / 1000.0f
                }

                duration
            }
            return try {
                callInMainThread(callable)
            } catch (e: ExecutionException) {
                (-1).toFloat()
            } catch (e: InterruptedException) {
                (-1).toFloat()
            }
        }

        @JvmStatic
        fun setVideoVisible(index: Int, visible: Boolean) {
            val msg = Message()
            msg.what = VideoTaskSetVisible
            msg.arg1 = index
            if (visible) {
                msg.arg2 = 1
            } else {
                msg.arg2 = 0
            }
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun setVideoKeepRatioEnabled(index: Int, enable: Boolean) {
            val msg = Message()
            msg.what = VideoTaskKeepRatio
            msg.arg1 = index
            if (enable) {
                msg.arg2 = 1
            } else {
                msg.arg2 = 0
            }
            mVideoHandler!!.sendMessage(msg)
        }

        @JvmStatic
        fun setVolume(index: Int, volume: Float) {
            val msg = Message()
            msg.what = VideoTaskSetVolume
            msg.arg1 = index
            msg.arg2 = (volume * 10).toInt()
            mVideoHandler!!.sendMessage(msg)
        }
    }
}
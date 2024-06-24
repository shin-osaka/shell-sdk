/*
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (c) 2014-2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
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
package eggy.cocos2dx.lib

import android.app.AlertDialog
import android.content.Intent
import android.media.AudioManager
import android.media.MediaPlayer
import android.media.MediaPlayer.OnCompletionListener
import android.media.MediaPlayer.OnPreparedListener
import android.net.Uri
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.RelativeLayout
import eggy.manager.CocosAssetsMgr
import java.io.IOException

class Cocos2dxVideoView(activity: Cocos2dxActivity?, tag: Int) : SurfaceView(activity) {
    interface OnVideoEventListener {
        fun onVideoEvent(tag: Int, event: Int)
    }

    private enum class State {
        IDLE, ERROR, INITIALIZED, PREPARING, PREPARED, STARTED, PAUSED, STOPPED, PLAYBACK_COMPLETED
    }

    private val TAG = "Cocos2dxVideoView"
    private var mVideoUri: Uri? = null
    private var mDuration = 0
    private var mCurrentState = State.IDLE
    private var mSurfaceHolder: SurfaceHolder? = null
    private var mMediaPlayer: MediaPlayer? = null
    private var mVideoWidth = 0
    private var mVideoHeight = 0
    private var mOnVideoEventListener: OnVideoEventListener? = null
    private val mSeekWhenPrepared = 0
    protected var mCocos2dxActivity: Cocos2dxActivity? = null
    protected var mViewLeft = 0
    protected var mViewTop = 0
    protected var mViewWidth = 0
    protected var mViewHeight = 0
    protected var mVisibleLeft = 0
    protected var mVisibleTop = 0
    protected var mVisibleWidth = 0
    protected var mVisibleHeight = 0
    protected var mFullScreenEnabled = false
    protected var mFullScreenWidth = 0
    protected var mFullScreenHeight = 0
    private var mIsAssetRouse = false
    private var mVideoFilePath: String? = null
    private var mViewTag = 0
    private var mKeepRatio = false
    private var mMetaUpdated = false
    private var mPositionBeforeRelease = 0
    fun setVideoRect(left: Int, top: Int, maxWidth: Int, maxHeight: Int) {
        if (mViewLeft == left && mViewTop == top && mViewWidth == maxWidth && mViewHeight == maxHeight) return
        mViewLeft = left
        mViewTop = top
        mViewWidth = maxWidth
        mViewHeight = maxHeight
        fixSize(mViewLeft, mViewTop, mViewWidth, mViewHeight)
    }

    fun setFullScreenEnabled(enabled: Boolean) {
        if (mFullScreenEnabled != enabled) {
            mFullScreenEnabled = enabled
            fixSize()
        }
    }

    fun setVolume(volume: Float) {
        if (mMediaPlayer != null) {
            mMediaPlayer!!.setVolume(volume, volume)
        }
    }

    fun setKeepRatio(enabled: Boolean) {
        mKeepRatio = enabled
        fixSize()
    }

    fun setVideoURL(url: String?) {
        mIsAssetRouse = false
        setVideoURI(Uri.parse(url), null)
    }

    fun setVideoFileName(p: String) {
        var path = p
        if (path.startsWith(AssetResourceRoot)) {
            path = path.substring(AssetResourceRoot.length)
        }
        if (path.startsWith("/")) {
            mIsAssetRouse = false
            setVideoURI(Uri.parse(path), null)
        } else {
            mVideoFilePath = path
            mIsAssetRouse = true
            setVideoURI(Uri.parse(path), null)
        }
    }

    val currentPosition: Int
        get() = if (!((mCurrentState == State.ERROR) or (
                    mMediaPlayer == null))
        ) {
            mMediaPlayer!!.currentPosition
        } else -1
    val duration: Int
        get() {
            if (!(mCurrentState == State.IDLE || mCurrentState == State.ERROR || mCurrentState == State.INITIALIZED || mMediaPlayer == null)) {
                mDuration = mMediaPlayer!!.duration
            }
            return mDuration
        }

    /**
     * Register a callback to be invoked when some video event triggered.
     *
     * @param l The callback that will be run
     */
    fun setVideoViewEventListener(l: OnVideoEventListener?) {
        mOnVideoEventListener = l
    }


    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        setMeasuredDimension(mVisibleWidth, mVisibleHeight)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (event.action and MotionEvent.ACTION_MASK == MotionEvent.ACTION_UP) {
            sendEvent(EVENT_CLICKED)
        }
        return true
    }

    fun stop() {
        if (!(mCurrentState == State.IDLE || mCurrentState == State.INITIALIZED || mCurrentState == State.ERROR || mCurrentState == State.STOPPED)
            && mMediaPlayer != null
        ) {
            mCurrentState = State.STOPPED
            mMediaPlayer!!.stop()
            sendEvent(EVENT_STOPPED)
            try {
                mMediaPlayer!!.prepare()
                showFirstFrame()
            } catch (_: Exception) {
            }
        }
    }

    fun stopPlayback() {
        release()
    }

    fun start() {
        if ((mCurrentState == State.PREPARED || mCurrentState == State.PAUSED || mCurrentState == State.PLAYBACK_COMPLETED) &&
            mMediaPlayer != null
        ) {
            mCurrentState = State.STARTED
            mMediaPlayer!!.start()
            sendEvent(EVENT_PLAYING)
        }
    }

    fun pause() {
        if ((mCurrentState == State.STARTED || mCurrentState == State.PLAYBACK_COMPLETED) &&
            mMediaPlayer != null
        ) {
            mCurrentState = State.PAUSED
            mMediaPlayer!!.pause()
            sendEvent(EVENT_PAUSED)
        }
    }

    fun seekTo(ms: Int) {
        if (mCurrentState == State.IDLE || mCurrentState == State.INITIALIZED || mCurrentState == State.STOPPED || mCurrentState == State.ERROR || mMediaPlayer == null) {
            return
        }
        mMediaPlayer!!.seekTo(ms)
    }

    fun fixSize() {
        if (mFullScreenEnabled) {
            mFullScreenWidth = mCocos2dxActivity!!.gLSurfaceView!!.width
            mFullScreenHeight = mCocos2dxActivity!!.gLSurfaceView!!.height
            fixSize(0, 0, mFullScreenWidth, mFullScreenHeight)
        } else {
            fixSize(mViewLeft, mViewTop, mViewWidth, mViewHeight)
        }
    }

    fun fixSize(left: Int, top: Int, width: Int, height: Int) {
        if (mVideoWidth == 0 || mVideoHeight == 0) {
            mVisibleLeft = left
            mVisibleTop = top
            mVisibleWidth = width
            mVisibleHeight = height
        } else if (width != 0 && height != 0) {
            if (mKeepRatio && !mFullScreenEnabled) {
                if (mVideoWidth * height > width * mVideoHeight) {
                    mVisibleWidth = width
                    mVisibleHeight = width * mVideoHeight / mVideoWidth
                } else if (mVideoWidth * height < width * mVideoHeight) {
                    mVisibleWidth = height * mVideoWidth / mVideoHeight
                    mVisibleHeight = height
                }
                mVisibleLeft = left + (width - mVisibleWidth) / 2
                mVisibleTop = top + (height - mVisibleHeight) / 2
            } else {
                mVisibleLeft = left
                mVisibleTop = top
                mVisibleWidth = width
                mVisibleHeight = height
            }
        } else {
            mVisibleLeft = left
            mVisibleTop = top
            mVisibleWidth = mVideoWidth
            mVisibleHeight = mVideoHeight
        }
        holder.setFixedSize(mVisibleWidth, mVisibleHeight)
        val lParams = RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT,
            RelativeLayout.LayoutParams.MATCH_PARENT
        )
        lParams.leftMargin = mVisibleLeft
        lParams.topMargin = mVisibleTop
        layoutParams = lParams
    }

    fun resolveAdjustedSize(desiredSize: Int, measureSpec: Int): Int {
        var result = desiredSize
        val specMode = MeasureSpec.getMode(measureSpec)
        val specSize = MeasureSpec.getSize(measureSpec)
        when (specMode) {
            MeasureSpec.UNSPECIFIED ->                 /* Parent says we can be as big as we want. Just don't be larger
                 * than max size imposed on ourselves.
                 */result = desiredSize

            MeasureSpec.AT_MOST ->                 /* Parent says we can be as big as we want, up to specSize.
                 * Don't be larger than specSize, and don't be larger than
                 * the max size imposed on ourselves.
                 */result = Math.min(desiredSize, specSize)

            MeasureSpec.EXACTLY -> result = specSize
        }
        return result
    }

    private fun initVideoView() {
        mVideoWidth = 0
        mVideoHeight = 0
        holder.addCallback(mSHCallback)
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS)
        isFocusable = true
        isFocusableInTouchMode = true
        mCurrentState = State.IDLE
    }

    /**
     * @hide
     */
    private fun setVideoURI(uri: Uri, headers: Map<String, String>?) {
        mVideoUri = uri
        mVideoWidth = 0
        mVideoHeight = 0
    }

    private fun openVideo() {
        if (mSurfaceHolder == null) {
            return
        }
        if (mIsAssetRouse) {
            if (mVideoFilePath == null) return
        } else if (mVideoUri == null) {
            return
        }
        pausePlaybackService()
        try {
            mMediaPlayer = MediaPlayer()
            mMediaPlayer!!.setOnPreparedListener(mPreparedListener)
            mMediaPlayer!!.setOnCompletionListener(mCompletionListener)
            mMediaPlayer!!.setOnErrorListener(mErrorListener)
            mMediaPlayer!!.setDisplay(mSurfaceHolder)
            mMediaPlayer!!.setAudioStreamType(AudioManager.STREAM_MUSIC)
            mMediaPlayer!!.setScreenOnWhilePlaying(true)
            if (mIsAssetRouse) {
                val afd = CocosAssetsMgr.getAssets(mCocos2dxActivity!!).openFd(
                    mVideoFilePath!!
                )
                mMediaPlayer!!.setDataSource(afd.fileDescriptor, afd.startOffset, afd.length)
            } else {
                mMediaPlayer!!.setDataSource(mVideoUri.toString())
            }
            mCurrentState = State.INITIALIZED
            mMediaPlayer!!.prepare()
            showFirstFrame()
        } catch (ex: IOException) {
            mCurrentState = State.ERROR
            mErrorListener.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0)
            return
        } catch (ex: IllegalArgumentException) {
            mCurrentState = State.ERROR
            mErrorListener.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0)
            return
        }
    }

    private var mPreparedListener = OnPreparedListener { mp ->
        mVideoWidth = mp.videoWidth
        mVideoHeight = mp.videoHeight
        if (mVideoWidth != 0 && mVideoHeight != 0) {
            fixSize()
        }
        if (!mMetaUpdated) {
            sendEvent(EVENT_META_LOADED)
            sendEvent(EVENT_READY_TO_PLAY)
            mMetaUpdated = true
        }
        mCurrentState = State.PREPARED
    }
    private val mCompletionListener = OnCompletionListener {
        mCurrentState = State.PLAYBACK_COMPLETED
        sendEvent(EVENT_COMPLETED)
    }
    private val mErrorListener = MediaPlayer.OnErrorListener { _, frameworkErr, _ ->
        mCurrentState = State.ERROR

        /* Otherwise, pop up an error dialog so the user knows that
             * something bad has happened. Only try and pop up the dialog
             * if we're attached to a window. When we're going away and no
             * longer have a window, don't bother showing the user an error.
             */if (windowToken != null) {
        val r = mCocos2dxActivity!!.resources
        val messageId =
            if (frameworkErr == MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
                r.getIdentifier(
                    "VideoView_error_text_invalid_progressive_playback",
                    "string",
                    "android"
                )
            } else {
                r.getIdentifier("VideoView_error_text_unknown", "string", "android")
            }
        val titleId = r.getIdentifier("VideoView_error_title", "string", "android")
        val buttonStringId = r.getIdentifier("VideoView_error_button", "string", "android")
        AlertDialog.Builder(mCocos2dxActivity)
            .setTitle(r.getString(titleId))
            .setMessage(messageId)
            .setPositiveButton(
                r.getString(buttonStringId)
            ) { _, _ -> /* If we get here, there is no onError listener, so
                                         * at least inform them that the video is over.
                                         */
                sendEvent(EVENT_COMPLETED)
            }
            .setCancelable(false)
            .show()
    }
        true
    }
    var mSHCallback: SurfaceHolder.Callback = object : SurfaceHolder.Callback {
        override fun surfaceChanged(holder: SurfaceHolder, format: Int, w: Int, h: Int) {}
        override fun surfaceCreated(holder: SurfaceHolder) {
            mSurfaceHolder = holder
            openVideo()
            if (mPositionBeforeRelease > 0) mMediaPlayer!!.seekTo(mPositionBeforeRelease)
        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            mSurfaceHolder = null
            mPositionBeforeRelease = currentPosition
            release()
        }
    }

    init {
        mViewTag = tag
        mCocos2dxActivity = activity
        initVideoView()
    }

    /*
     * release the media player in any state
     */
    private fun release() {
        if (mMediaPlayer != null) {
            mMediaPlayer!!.release()
            mMediaPlayer = null
        }
    }

    private fun showFirstFrame() {
        mMediaPlayer!!.seekTo(1)
    }

    private fun sendEvent(event: Int) {
        if (mOnVideoEventListener != null) {
            mOnVideoEventListener!!.onVideoEvent(mViewTag, event)
        }
    }

    private fun pausePlaybackService() {
        val i = Intent("com.android.music.musicservicecommand")
        i.putExtra("command", "pause")
        mCocos2dxActivity!!.sendBroadcast(i)
    }

    companion object {
        private const val AssetResourceRoot = "@assets/"
        private const val EVENT_PLAYING = 0
        private const val EVENT_PAUSED = 1
        private const val EVENT_STOPPED = 2
        private const val EVENT_COMPLETED = 3
        private const val EVENT_META_LOADED = 4
        private const val EVENT_CLICKED = 5
        private const val EVENT_READY_TO_PLAY = 6
    }
}
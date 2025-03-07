/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package eggy.cocos2dx.lib;

import android.opengl.GLSurfaceView;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Cocos2dxRenderer implements GLSurfaceView.Renderer {
    private final static String TAG = "Cocos2dxRenderer";

    private final static long NANOSECONDSPERSECOND = 1000000000L;
    private final static long NANOSECONDSPERMICROSECOND = 1000000;

    private static final long INTERVAL_60_FPS = (long) (1.0 / 60 * Cocos2dxRenderer.NANOSECONDSPERSECOND);
    private static long sAnimationInterval = INTERVAL_60_FPS;
    private static WeakReference<Cocos2dxRenderer> sRenderer;

    private long mLastTickInNanoSeconds;
    private int mScreenWidth;
    private int mScreenHeight;
    private boolean mNativeInitCompleted = false;
    private boolean mNeedShowFPS = false;
    private String mDefaultResourcePath = "";
    private long mOldNanoTime = 0;
    private long mFrameCount = 0;
    private boolean mNeedToPause = false;

    public static void setPreferredFramesPerSecond(int fps) {
        Cocos2dxRenderer.sAnimationInterval = (long) (1.0 / fps * Cocos2dxRenderer.NANOSECONDSPERSECOND);
    }

    public void setScreenWidthAndHeight(final int surfaceWidth, final int surfaceHeight) {
        this.mScreenWidth = surfaceWidth;
        this.mScreenHeight = surfaceHeight;
    }

    public void setDefaultResourcePath(String path) {
        if (path == null)
            return;
        mDefaultResourcePath = path;
    }

    public void showFPS() {
        mNeedShowFPS = true;
    }

    public interface OnGameEngineInitializedListener {
        void onGameEngineInitialized();
    }

    private OnGameEngineInitializedListener mGameEngineInitializedListener;

    public void setOnGameEngineInitializedListener(OnGameEngineInitializedListener listener) {
        mGameEngineInitializedListener = listener;
    }

    @Override
    public void onSurfaceCreated(final GL10 GL10, final EGLConfig EGLConfig) {
        mNativeInitCompleted = false;
        Cocos2dxRenderer.nativeInit(this.mScreenWidth, this.mScreenHeight, mDefaultResourcePath);
        mOldNanoTime = System.nanoTime();
        this.mLastTickInNanoSeconds = System.nanoTime();
        mNativeInitCompleted = true;
        if (mGameEngineInitializedListener != null) {
            Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mGameEngineInitializedListener.onGameEngineInitialized();
                }
            });
        }
    }

    @Override
    public void onSurfaceChanged(final GL10 GL10, final int width, final int height) {
        Cocos2dxRenderer.nativeOnSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(final GL10 gl) {
        if (mNeedToPause)
            return;

        if (mNeedShowFPS) {
            ++mFrameCount;
            long nowFpsTime = System.nanoTime();
            long fpsTimeInterval = nowFpsTime - mOldNanoTime;
            if (fpsTimeInterval > 1000000000L) {
                double frameRate = 1000000000.0 * mFrameCount / fpsTimeInterval;
                Cocos2dxHelper.OnGameInfoUpdatedListener listener = Cocos2dxHelper.getOnGameInfoUpdatedListener();
                if (listener != null) {
                    listener.onFPSUpdated((float) frameRate);
                }
                mFrameCount = 0;
                mOldNanoTime = System.nanoTime();
            }
        }
        /*
         * No need to use algorithm in default(60 FPS) situation,
         * since onDrawFrame() was called by system 60 times per second by default.
         */
        if (sAnimationInterval <= INTERVAL_60_FPS) {
            Cocos2dxRenderer.nativeRender();
        } else {
            final long now = System.nanoTime();
            final long interval = now - this.mLastTickInNanoSeconds;

            if (interval < Cocos2dxRenderer.sAnimationInterval) {
                try {
                    Thread.sleep((Cocos2dxRenderer.sAnimationInterval - interval)
                            / Cocos2dxRenderer.NANOSECONDSPERMICROSECOND);
                } catch (final Exception e) {
                }
            }
            /*
             * Render time MUST be counted in, or the FPS will slower than appointed.
             */
            this.mLastTickInNanoSeconds = System.nanoTime();
            Cocos2dxRenderer.nativeRender();
        }
    }

    private static native void nativeTouchesBegin(final int id, final float x, final float y);

    private static native void nativeTouchesEnd(final int id, final float x, final float y);

    private static native void nativeTouchesMove(final int[] ids, final float[] xs, final float[] ys);

    private static native void nativeTouchesCancel(final int[] ids, final float[] xs, final float[] ys);

    private static native boolean nativeKeyEvent(final int keyCode, boolean isPressed);

    private static native void nativeRender();

    private static native void nativeInit(final int width, final int height, final String resourcePath);

    private static native void nativeOnSurfaceChanged(final int width, final int height);

    private static native void nativeOnPause();

    private static native void nativeOnResume();

    public void setPauseInMainThread(boolean value) {
        mNeedToPause = value;
    }

    public void handleActionDown(final int id, final float x, final float y) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeTouchesBegin(id, x, y);
    }

    public void handleActionUp(final int id, final float x, final float y) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeTouchesEnd(id, x, y);
    }

    public void handleActionCancel(final int[] ids, final float[] xs, final float[] ys) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeTouchesCancel(ids, xs, ys);
    }

    public void handleActionMove(final int[] ids, final float[] xs, final float[] ys) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeTouchesMove(ids, xs, ys);
    }

    public void handleKeyDown(final int keyCode) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeKeyEvent(keyCode, true);
    }

    public void handleKeyUp(final int keyCode) {
        if (!mNativeInitCompleted)
            return;

        Cocos2dxRenderer.nativeKeyEvent(keyCode, false);
    }

    public void handleOnPause() {
        /**
         * onPause may be invoked before onSurfaceCreated,
         * and engine will be initialized correctly after
         * onSurfaceCreated is invoked. Can not invoke any
         * native method before onSurfaceCreated is invoked
         */
        if (!mNativeInitCompleted)
            return;

        Cocos2dxHelper.onEnterBackground();
        Cocos2dxRenderer.nativeOnPause();
    }

    public void handleOnResume() {
        Cocos2dxHelper.onEnterForeground();
        Cocos2dxRenderer.nativeOnResume();
    }

    private static native void nativeInsertText(final String text);

    private static native void nativeDeleteBackward();

    private static native String nativeGetContentText();

    public void handleInsertText(final String text) {
        Cocos2dxRenderer.nativeInsertText(text);
    }

    public void handleDeleteBackward() {
        Cocos2dxRenderer.nativeDeleteBackward();
    }

    public String getContentText() {
        return Cocos2dxRenderer.nativeGetContentText();
    }

}

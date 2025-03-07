/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org
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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.preference.PreferenceManager.OnActivityResultListener;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import eggy.cocos2dx.custom.Utils;
import eggy.cocos2dx.lib.Cocos2dxHelper.Cocos2dxHelperListener;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public abstract class Cocos2dxActivity extends Activity implements Cocos2dxHelperListener {

    private final static String TAG = Cocos2dxActivity.class.getSimpleName();

    private static Cocos2dxActivity sContext = null;

    protected RelativeLayout mFrameLayout = null;

    private Cocos2dxGLSurfaceView mGLSurfaceView = null;
    private int[] mGLContextAttrs = null;
    private Cocos2dxHandler mHandler = null;
    private Cocos2dxVideoHelper mVideoHelper = null;
    private Cocos2dxWebViewHelper mWebViewHelper = null;
    private boolean hasFocus = false;
    private Cocos2dxEditBox mEditBox = null;
    private boolean gainAudioFocus = false;
    private boolean paused = true;

    private LinearLayout mLinearLayoutForDebugView;
    private TextView mFPSTextView;
    private TextView mJSBInvocationTextView;
    private TextView mGLOptModeTextView;
    private TextView mGameInfoTextView_0;
    private TextView mGameInfoTextView_1;
    private TextView mGameInfoTextView_2;

    public class Cocos2dxEGLConfigChooser implements GLSurfaceView.EGLConfigChooser {
        protected int[] configAttribs;

        public Cocos2dxEGLConfigChooser(int redSize, int greenSize, int blueSize, int alphaSize, int depthSize,
                int stencilSize) {
            configAttribs = new int[] { redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize };
        }

        public Cocos2dxEGLConfigChooser(int[] attribs) {
            configAttribs = attribs;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                EGLConfig config, int attribute, int defaultValue) {
            int[] value = new int[1];
            if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
                return value[0];
            }
            return defaultValue;
        }

        class ConfigValue implements Comparable<ConfigValue> {
            public EGLConfig config = null;
            public int[] configAttribs = null;
            public int value = 0;

            private void calcValue() {
                if (configAttribs[4] > 0) {
                    value = value + (1 << 29) + ((configAttribs[4] % 64) << 6);
                }
                if (configAttribs[5] > 0) {
                    value = value + (1 << 28) + ((configAttribs[5] % 64));
                }
                if (configAttribs[3] > 0) {
                    value = value + (1 << 30) + ((configAttribs[3] % 16) << 24);
                }
                if (configAttribs[1] > 0) {
                    value = value + ((configAttribs[1] % 16) << 20);
                }
                if (configAttribs[2] > 0) {
                    value = value + ((configAttribs[2] % 16) << 16);
                }
                if (configAttribs[0] > 0) {
                    value = value + ((configAttribs[0] % 16) << 12);
                }
            }

            public ConfigValue(int[] attribs) {
                configAttribs = attribs;
                calcValue();
            }

            public ConfigValue(EGL10 egl, EGLDisplay display, EGLConfig config) {
                this.config = config;
                configAttribs = new int[6];
                configAttribs[0] = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0);
                configAttribs[1] = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0);
                configAttribs[2] = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0);
                configAttribs[3] = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0);
                configAttribs[4] = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0);
                configAttribs[5] = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0);
                calcValue();
            }

            @Override
            public int compareTo(ConfigValue another) {
                if (value < another.value) {
                    return -1;
                } else if (value > another.value) {
                    return 1;
                } else {
                    return 0;
                }
            }

            @Override
            public String toString() {
                return "{ color: " + configAttribs[3] + configAttribs[2] + configAttribs[1] + configAttribs[0] +
                        "; depth: " + configAttribs[4] + "; stencil: " + configAttribs[5] + ";}";
            }
        }

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            int[] EGLattribs = {
                    EGL10.EGL_RED_SIZE, configAttribs[0],
                    EGL10.EGL_GREEN_SIZE, configAttribs[1],
                    EGL10.EGL_BLUE_SIZE, configAttribs[2],
                    EGL10.EGL_ALPHA_SIZE, configAttribs[3],
                    EGL10.EGL_DEPTH_SIZE, configAttribs[4],
                    EGL10.EGL_STENCIL_SIZE, configAttribs[5],
                    EGL10.EGL_RENDERABLE_TYPE, 4, // EGL_OPENGL_ES2_BIT
                    EGL10.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] numConfigs = new int[1];
            boolean eglChooseResult = egl.eglChooseConfig(display, EGLattribs, configs, 1, numConfigs);
            if (eglChooseResult && numConfigs[0] > 0) {
                return configs[0];
            }

            int[] EGLV2attribs = {
                    EGL10.EGL_RENDERABLE_TYPE, 4, // EGL_OPENGL_ES2_BIT
                    EGL10.EGL_NONE
            };
            eglChooseResult = egl.eglChooseConfig(display, EGLV2attribs, null, 0, numConfigs);
            if (eglChooseResult && numConfigs[0] > 0) {
                int num = numConfigs[0];
                ConfigValue[] cfgVals = new ConfigValue[num];

                configs = new EGLConfig[num];
                egl.eglChooseConfig(display, EGLV2attribs, configs, num, numConfigs);
                for (int i = 0; i < num; ++i) {
                    cfgVals[i] = new ConfigValue(egl, display, configs[i]);
                }

                ConfigValue e = new ConfigValue(configAttribs);
                int lo = 0;
                int hi = num;
                int mi;
                while (lo < hi - 1) {
                    mi = (lo + hi) / 2;
                    if (e.compareTo(cfgVals[mi]) < 0) {
                        hi = mi;
                    } else {
                        lo = mi;
                    }
                }
                if (lo != num - 1) {
                    lo = lo + 1;
                }
                return cfgVals[lo].config;
            }

            return null;
        }

    }

    public Cocos2dxGLSurfaceView getGLSurfaceView() {
        return mGLSurfaceView;
    }

    public static Context getContext() {
        return sContext;
    }

    public void init() {
        ViewGroup.LayoutParams frameLayoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        mFrameLayout = new RelativeLayout(this);
        mFrameLayout.setLayoutParams(frameLayoutParams);

        Cocos2dxRenderer renderer = this.addSurfaceView();
        this.addDebugInfo(renderer);

        mEditBox = new Cocos2dxEditBox(this, mFrameLayout);

        setContentView(mFrameLayout);
    }

    public void setKeepScreenOn(boolean value) {
        final boolean newValue = value;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mGLSurfaceView.setKeepScreenOn(newValue);
            }
        });
    }

    public void setEnableAudioFocusGain(boolean value) {
        if (gainAudioFocus != value) {
            if (!paused) {
                if (value)
                    Cocos2dxAudioFocusManager.registerAudioFocusListener(this);
                else
                    Cocos2dxAudioFocusManager.unregisterAudioFocusListener(this);
            }
            gainAudioFocus = value;
        }
    }

    public Cocos2dxGLSurfaceView onCreateView() {
        Cocos2dxGLSurfaceView glSurfaceView = new Cocos2dxGLSurfaceView(this);
        if (this.mGLContextAttrs[3] > 0)
            glSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);

        Cocos2dxEGLConfigChooser chooser = new Cocos2dxEGLConfigChooser(this.mGLContextAttrs);
        glSurfaceView.setEGLConfigChooser(chooser);

        return glSurfaceView;
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!isTaskRoot()) {
            finish();
            return;
        }

        Utils.setActivity(this);
        Utils.hideVirtualButton();
        Cocos2dxHelper.registerBatteryLevelReceiver(this);
        onLoadNativeLibraries();

        sContext = this;
        this.mHandler = new Cocos2dxHandler(this);

        Cocos2dxHelper.init(this);
        CanvasRenderingContext2DImpl.init(this);

        this.mGLContextAttrs = getGLContextAttrs();
        this.init();

        if (mVideoHelper == null) {
            mVideoHelper = new Cocos2dxVideoHelper(this, mFrameLayout);
        }

        if (mWebViewHelper == null) {
            mWebViewHelper = new Cocos2dxWebViewHelper(mFrameLayout);
        }

        Window window = this.getWindow();
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN);
        this.setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    @Override
    protected void onResume() {
        paused = false;
        super.onResume();
        if (gainAudioFocus)
            Cocos2dxAudioFocusManager.registerAudioFocusListener(this);
        Utils.hideVirtualButton();
        resumeIfHasFocus();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        this.hasFocus = hasFocus;
        resumeIfHasFocus();
    }

    private void resumeIfHasFocus() {
        if (hasFocus && !paused) {
            Utils.hideVirtualButton();
            Cocos2dxHelper.onResume();
            mGLSurfaceView.onResume();
        }
    }

    @Override
    protected void onPause() {
        paused = true;
        super.onPause();
        if (gainAudioFocus)
            Cocos2dxAudioFocusManager.unregisterAudioFocusListener(this);
        Cocos2dxHelper.onPause();
        mGLSurfaceView.onPause();
    }

    @Override
    protected void onDestroy() {
        if (gainAudioFocus)
            Cocos2dxAudioFocusManager.unregisterAudioFocusListener(this);
        Cocos2dxHelper.unregisterBatteryLevelReceiver(this);
        CanvasRenderingContext2DImpl.destroy();

        super.onDestroy();

        if (mGLSurfaceView != null) {
            Cocos2dxHelper.terminateProcess();
        }
    }

    @Override
    public void showDialog(final String pTitle, final String pMessage) {
        Message msg = new Message();
        msg.what = Cocos2dxHandler.HANDLER_SHOW_DIALOG;
        msg.obj = new Cocos2dxHandler.DialogMessage(pTitle, pMessage);
        this.mHandler.sendMessage(msg);
    }

    @Override
    public void runOnGLThread(final Runnable runnable) {
        this.mGLSurfaceView.queueEvent(runnable);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        for (OnActivityResultListener listener : Cocos2dxHelper.getOnActivityResultListeners()) {
            listener.onActivityResult(requestCode, resultCode, data);
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    protected void onLoadNativeLibraries() {
        try {
            ApplicationInfo ai = getPackageManager().getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
            Bundle bundle = ai.metaData;
            String libName = bundle.getString("android.app.lib_name");
            System.loadLibrary(libName);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private Cocos2dxRenderer addSurfaceView() {
        this.mGLSurfaceView = this.onCreateView();
        this.mGLSurfaceView.setPreserveEGLContextOnPause(true);
        mGLSurfaceView.setBackgroundColor(Color.TRANSPARENT);
        if (isAndroidEmulator())
            this.mGLSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        Cocos2dxRenderer renderer = new Cocos2dxRenderer();
        this.mGLSurfaceView.setCocos2dxRenderer(renderer);

        mFrameLayout.addView(this.mGLSurfaceView);

        this.mGLSurfaceView.requestFocus();
        return renderer;
    }

    private void addDebugInfo(Cocos2dxRenderer renderer) {
        LinearLayout.LayoutParams linearLayoutParam = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        linearLayoutParam.setMargins(30, 0, 0, 0);
        Cocos2dxHelper.setOnGameInfoUpdatedListener(new Cocos2dxHelper.OnGameInfoUpdatedListener() {
            @Override
            public void onFPSUpdated(float fps) {
                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mFPSTextView != null) {
                            mFPSTextView.setText("FPS: " + (int) Math.ceil(fps));
                        }
                    }
                });
            }

            @Override
            public void onJSBInvocationCountUpdated(int count) {
                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mJSBInvocationTextView != null) {
                            mJSBInvocationTextView.setText("JSB: " + count);
                        }
                    }
                });
            }

            @Override
            public void onOpenDebugView() {
                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mLinearLayoutForDebugView != null || mFrameLayout == null) {
                            return;
                        }

                        mLinearLayoutForDebugView = new LinearLayout(Cocos2dxActivity.this);
                        mLinearLayoutForDebugView.setOrientation(LinearLayout.VERTICAL);
                        mFrameLayout.addView(mLinearLayoutForDebugView);

                        mFPSTextView = new TextView(Cocos2dxActivity.this);
                        mFPSTextView.setBackgroundColor(Color.RED);
                        mFPSTextView.setTextColor(Color.WHITE);
                        mLinearLayoutForDebugView.addView(mFPSTextView, linearLayoutParam);

                        mJSBInvocationTextView = new TextView(Cocos2dxActivity.this);
                        mJSBInvocationTextView.setBackgroundColor(Color.GREEN);
                        mJSBInvocationTextView.setTextColor(Color.WHITE);
                        mLinearLayoutForDebugView.addView(mJSBInvocationTextView, linearLayoutParam);

                        mGLOptModeTextView = new TextView(Cocos2dxActivity.this);
                        mGLOptModeTextView.setBackgroundColor(Color.BLUE);
                        mGLOptModeTextView.setTextColor(Color.WHITE);
                        mGLOptModeTextView.setText("GL Opt: Enabled");
                        mLinearLayoutForDebugView.addView(mGLOptModeTextView, linearLayoutParam);

                        mGameInfoTextView_0 = new TextView(Cocos2dxActivity.this);
                        mGameInfoTextView_0.setBackgroundColor(Color.RED);
                        mGameInfoTextView_0.setTextColor(Color.WHITE);
                        mLinearLayoutForDebugView.addView(mGameInfoTextView_0, linearLayoutParam);

                        mGameInfoTextView_1 = new TextView(Cocos2dxActivity.this);
                        mGameInfoTextView_1.setBackgroundColor(Color.GREEN);
                        mGameInfoTextView_1.setTextColor(Color.WHITE);
                        mLinearLayoutForDebugView.addView(mGameInfoTextView_1, linearLayoutParam);

                        mGameInfoTextView_2 = new TextView(Cocos2dxActivity.this);
                        mGameInfoTextView_2.setBackgroundColor(Color.BLUE);
                        mGameInfoTextView_2.setTextColor(Color.WHITE);
                        mLinearLayoutForDebugView.addView(mGameInfoTextView_2, linearLayoutParam);
                    }
                });

                renderer.showFPS();
            }

            @Override
            public void onDisableBatchGLCommandsToNative() {
                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mGLOptModeTextView != null) {
                            mGLOptModeTextView.setText("GL Opt: Disabled");
                        }
                    }
                });
            }

            @Override
            public void onGameInfoUpdated_0(final String text) {

                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mGameInfoTextView_0 != null) {
                            mGameInfoTextView_0.setText(text);
                        }
                    }
                });
            }

            @Override
            public void onGameInfoUpdated_1(String text) {

                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mGameInfoTextView_1 != null) {
                            mGameInfoTextView_1.setText(text);
                        }
                    }
                });
            }

            @Override
            public void onGameInfoUpdated_2(String text) {

                Cocos2dxActivity.this.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (mGameInfoTextView_2 != null) {
                            mGameInfoTextView_2.setText(text);
                        }
                    }
                });
            }
        });
    }

    private final static boolean isAndroidEmulator() {
        String model = Build.MODEL;
        String product = Build.PRODUCT;
        boolean isEmulator = false;
        if (product != null) {
            isEmulator = product.equals("sdk") || product.contains("_sdk") || product.contains("sdk_");
        }
        return isEmulator;
    }

    private static native int[] getGLContextAttrs();
}

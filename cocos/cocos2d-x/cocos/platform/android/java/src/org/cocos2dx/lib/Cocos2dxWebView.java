/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

package eggy.cocos2dx.lib;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;
import android.view.Gravity;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.RelativeLayout;

import java.lang.reflect.Method;
import java.net.URI;
import java.util.concurrent.CountDownLatch;

class ShouldStartLoadingWorker implements Runnable {
    private final CountDownLatch mLatch;
    private final boolean[] mResult;
    private final int mViewTag;
    private final String mUrlString;

    ShouldStartLoadingWorker(CountDownLatch latch, boolean[] result, int viewTag, String urlString) {
        this.mLatch = latch;
        this.mResult = result;
        this.mViewTag = viewTag;
        this.mUrlString = urlString;
    }

    @Override
    public void run() {
        this.mResult[0] = Cocos2dxWebViewHelper._shouldStartLoading(mViewTag, mUrlString);
        this.mLatch.countDown(); // notify that result is ready
    }
}

public class Cocos2dxWebView extends WebView {
    private static final String TAG = Cocos2dxWebViewHelper.class.getSimpleName();

    private final int mViewTag;
    private String mJSScheme;

    public Cocos2dxWebView(Context context) {
        this(context, -1);
    }

    @SuppressLint("SetJavaScriptEnabled")
    public Cocos2dxWebView(Context context, int viewTag) {
        super(context);
        this.mViewTag = viewTag;
        this.mJSScheme = "";

        this.setFocusable(true);
        this.setFocusableInTouchMode(true);

        this.getSettings().setSupportZoom(false);

        this.getSettings().setDomStorageEnabled(true);
        this.getSettings().setJavaScriptEnabled(true);

        try {
            Method method = this.getClass().getMethod("removeJavascriptInterface", String.class);
            method.invoke(this, "searchBoxJavaBridge_");
        } catch (Exception e) {
        }

        this.setWebViewClient(new Cocos2dxWebViewClient());
        this.setWebChromeClient(new WebChromeClient());
    }

    public void setJavascriptInterfaceScheme(String scheme) {
        this.mJSScheme = scheme != null ? scheme : "";
    }

    public void setScalesPageToFit(boolean scalesPageToFit) {
        this.getSettings().setSupportZoom(scalesPageToFit);
    }

    class Cocos2dxWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, final String urlString) {
            Cocos2dxActivity activity = (Cocos2dxActivity) getContext();

            try {
                URI uri = URI.create(urlString);
                if (uri != null && uri.getScheme().equals(mJSScheme)) {
                    activity.runOnGLThread(new Runnable() {
                        @Override
                        public void run() {
                            Cocos2dxWebViewHelper._onJsCallback(mViewTag, urlString);
                        }
                    });
                    return true;
                }
            } catch (Exception e) {
            }

            boolean[] result = new boolean[] { true };
            CountDownLatch latch = new CountDownLatch(1);

            activity.runOnGLThread(new ShouldStartLoadingWorker(latch, result, mViewTag, urlString));

            try {
                latch.await();
            } catch (InterruptedException ex) {
            }

            return result[0];
        }

        @Override
        public void onPageFinished(WebView view, final String url) {
            super.onPageFinished(view, url);
            Cocos2dxActivity activity = (Cocos2dxActivity) getContext();
            activity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    Cocos2dxWebViewHelper._didFinishLoading(mViewTag, url);
                }
            });
        }

        @Override
        public void onReceivedError(WebView view, int errorCode, String description, final String failingUrl) {
            super.onReceivedError(view, errorCode, description, failingUrl);
            Cocos2dxActivity activity = (Cocos2dxActivity) getContext();
            activity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    Cocos2dxWebViewHelper._didFailLoading(mViewTag, failingUrl);
                }
            });
        }
    }

    public void setWebViewRect(int left, int top, int maxWidth, int maxHeight) {
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.MATCH_PARENT);
        layoutParams.leftMargin = left;
        layoutParams.topMargin = top;
        layoutParams.width = maxWidth;
        layoutParams.height = maxHeight;
        this.setLayoutParams(layoutParams);
    }
}

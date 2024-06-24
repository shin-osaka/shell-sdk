/****************************************************************************
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated engine source code (the "Software"), a limited,
 * worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 * to use Cocos Creator solely to develop games on your target platforms. You shall
 * not use Cocos Creator software for developing other software or tools that's
 * used for developing games. You are not granted to publish, distribute,
 * sublicense, and/or sell copies of Cocos Creator.
 *
 * The software or tools in this License Agreement are licensed, not sold.
 * Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.
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

import android.annotation.SuppressLint
import android.content.Context
import android.webkit.WebChromeClient
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.RelativeLayout
import java.net.URI
import java.util.concurrent.CountDownLatch

internal class ShouldStartLoadingWorker(
    private val mLatch: CountDownLatch,
    private val mResult: BooleanArray,
    private val mViewTag: Int,
    private val mUrlString: String?
) : Runnable {
    override fun run() {
        mResult[0] = Cocos2dxWebViewHelper._shouldStartLoading(mViewTag, mUrlString)
        mLatch.countDown() // notify that result is ready
    }
}

class Cocos2dxWebView @SuppressLint("SetJavaScriptEnabled") constructor(
    context: Context?,
    private val mViewTag: Int
) : WebView(
    context!!
) {
    private var mJSScheme = ""

    constructor(context: Context?) : this(context, -1)

    init {
        this.isFocusable = true
        this.isFocusableInTouchMode = true
        this.settings.setSupportZoom(false)
        this.settings.domStorageEnabled = true
        this.settings.javaScriptEnabled = true
        try {
            val method = this.javaClass.getMethod("removeJavascriptInterface", String::class.java)
            method.invoke(this, "searchBoxJavaBridge_")
        } catch (_: Exception) {
        }
        this.webViewClient = Cocos2dxWebViewClient()
        this.webChromeClient = WebChromeClient()
    }

    fun setJavascriptInterfaceScheme(scheme: String?) {
        mJSScheme = scheme ?: ""
    }

    fun setScalesPageToFit(scalesPageToFit: Boolean) {
        this.settings.setSupportZoom(scalesPageToFit)
    }

    internal inner class Cocos2dxWebViewClient : WebViewClient() {
        override fun shouldOverrideUrlLoading(view: WebView?, urlString: String?): Boolean {
            val activity = context as Cocos2dxActivity
            try {
                val uri = URI.create(urlString)
                if (uri != null && uri.scheme == mJSScheme) {
                    activity.runOnGLThread {
                        Cocos2dxWebViewHelper._onJsCallback(
                            mViewTag,
                            urlString
                        )
                    }
                    return true
                }
            } catch (_: Exception) {
            }
            val result = booleanArrayOf(true)
            val latch = CountDownLatch(1)
            activity.runOnGLThread(ShouldStartLoadingWorker(latch, result, mViewTag, urlString))
            try {
                latch.await()
            } catch (_: InterruptedException) {
            }
            return result[0]
        }

        override fun onPageFinished(view: WebView?, url: String?) {
            super.onPageFinished(view, url)
            val activity = context as Cocos2dxActivity
            activity.runOnGLThread { Cocos2dxWebViewHelper._didFinishLoading(mViewTag, url) }
        }

        override fun onReceivedError(
            view: WebView?,
            errorCode: Int,
            description: String?,
            failingUrl: String?
        ) {
            super.onReceivedError(view, errorCode, description, failingUrl)
            val activity = context as Cocos2dxActivity
            activity.runOnGLThread { Cocos2dxWebViewHelper._didFailLoading(mViewTag, failingUrl) }
        }
    }

    fun setWebViewRect(left: Int, top: Int, maxWidth: Int, maxHeight: Int) {
        val layoutParams = RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT,
            RelativeLayout.LayoutParams.MATCH_PARENT
        )
        layoutParams.leftMargin = left
        layoutParams.topMargin = top
        layoutParams.width = maxWidth
        layoutParams.height = maxHeight
        setLayoutParams(layoutParams)
    }

    companion object {
        private val TAG = Cocos2dxWebViewHelper::class.java.simpleName
    }
}
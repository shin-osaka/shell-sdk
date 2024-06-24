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

import android.graphics.Color
import android.os.Handler
import android.os.Looper
import android.util.SparseArray
import android.view.View
import android.webkit.WebView
import android.widget.FrameLayout
import android.widget.RelativeLayout
import java.util.concurrent.Callable
import java.util.concurrent.ExecutionException
import java.util.concurrent.FutureTask

class Cocos2dxWebViewHelper(layout: RelativeLayout?) {
    init {
        sLayout = layout
        sHandler = Handler(Looper.myLooper()!!)
        sCocos2dxActivity = Cocos2dxActivity.context as Cocos2dxActivity
        webViews = SparseArray()
    }

    companion object {
        private val TAG = Cocos2dxWebViewHelper::class.java.simpleName
        private var sHandler: Handler? = null
        private var sCocos2dxActivity: Cocos2dxActivity? = null
        private var sLayout: RelativeLayout? = null
        private var webViews: SparseArray<Cocos2dxWebView>? = null
        private var viewTag = 0

        @JvmStatic
        private external fun shouldStartLoading(index: Int, message: String?): Boolean
        fun _shouldStartLoading(index: Int, message: String?): Boolean {
            return !shouldStartLoading(index, message)
        }

        @JvmStatic
        private external fun didFinishLoading(index: Int, message: String?)
        fun _didFinishLoading(index: Int, message: String?) {
            didFinishLoading(index, message)
        }

        @JvmStatic
        private external fun didFailLoading(index: Int, message: String?)
        fun _didFailLoading(index: Int, message: String?) {
            didFailLoading(index, message)
        }

        @JvmStatic
        private external fun onJsCallback(index: Int, message: String?)
        fun _onJsCallback(index: Int, message: String?) {
            onJsCallback(index, message)
        }

        @JvmStatic
        fun createWebView(): Int {
            val index = viewTag
            sCocos2dxActivity!!.runOnUiThread {
                val webView = Cocos2dxWebView(sCocos2dxActivity, index)
                val lParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.WRAP_CONTENT,
                    FrameLayout.LayoutParams.WRAP_CONTENT
                )
                sLayout!!.addView(webView, lParams)
                webViews!!.put(index, webView)
            }
            return viewTag++
        }

        @JvmStatic
        fun removeWebView(index: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                var webView = webViews!![index]
                if (webView != null) {
                    webViews!!.remove(index)
                    sLayout!!.removeView(webView)
                    webView.destroy()
                    webView = null
                }
            }
        }

        @JvmStatic
        fun setVisible(index: Int, visible: Boolean) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                if (webView != null) {
                    webView.visibility = if (visible) View.VISIBLE else View.GONE
                }
            }
        }

        @JvmStatic
        fun setWebViewRect(index: Int, left: Int, top: Int, maxWidth: Int, maxHeight: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.setWebViewRect(left, top, maxWidth, maxHeight)
            }
        }

        @JvmStatic
        fun setBackgroundTransparent(index: Int, isTransparent: Boolean) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                if (webView != null) {
                    webView.setBackgroundColor(if (isTransparent) Color.TRANSPARENT else Color.WHITE)
                    webView.setLayerType(WebView.LAYER_TYPE_SOFTWARE, null)
                }
            }
        }

        @JvmStatic
        fun setJavascriptInterfaceScheme(index: Int, scheme: String?) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.setJavascriptInterfaceScheme(scheme)
            }
        }

        @JvmStatic
        fun loadData(
            index: Int,
            data: String?,
            mimeType: String?,
            encoding: String?,
            baseURL: String?
        ) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.loadDataWithBaseURL(baseURL, data!!, mimeType, encoding, null)
            }
        }

        @JvmStatic
        fun loadHTMLString(index: Int, data: String?, baseUrl: String?) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.loadDataWithBaseURL(baseUrl, data!!, null, null, null)
            }
        }

        @JvmStatic
        fun loadUrl(index: Int, url: String?) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.loadUrl(url!!)
            }
        }

        @JvmStatic
        fun loadFile(index: Int, filePath: String?) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.loadUrl(filePath!!)
            }
        }

        @JvmStatic
        fun stopLoading(index: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.stopLoading()
            }
        }

        @JvmStatic
        fun reload(index: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.reload()
            }
        }

        @Throws(ExecutionException::class, InterruptedException::class)
        fun <T> callInMainThread(call: Callable<T>?): T {
            val task = FutureTask(call)
            sHandler!!.post(task)
            return task.get()
        }

        @JvmStatic
        fun canGoBack(index: Int): Boolean {
            val callable = Callable {
                val webView = webViews!![index]
                webView != null && webView.canGoBack()
            }
            return try {
                callInMainThread(callable)
            } catch (e: ExecutionException) {
                false
            } catch (e: InterruptedException) {
                false
            }
        }

        @JvmStatic
        fun canGoForward(index: Int): Boolean {
            val callable = Callable {
                val webView = webViews!![index]
                webView != null && webView.canGoForward()
            }
            return try {
                callInMainThread(callable)
            } catch (e: ExecutionException) {
                false
            } catch (e: InterruptedException) {
                false
            }
        }

        @JvmStatic
        fun goBack(index: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.goBack()
            }
        }

        @JvmStatic
        fun goForward(index: Int) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.goForward()
            }
        }

        @JvmStatic
        fun evaluateJS(index: Int, js: String?) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.loadUrl("javascript:$js")
            }
        }

        @JvmStatic
        fun setScalesPageToFit(index: Int, scalesPageToFit: Boolean) {
            sCocos2dxActivity!!.runOnUiThread {
                val webView = webViews!![index]
                webView?.setScalesPageToFit(scalesPageToFit)
            }
        }
    }
}
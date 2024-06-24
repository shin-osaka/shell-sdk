package eggy.game.core

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.webkit.JavascriptInterface
import androidx.annotation.Keep
import eggy.cocos2dx.lib.Cocos2dxHelper
import eggy.cocos2dx.lib.Cocos2dxJavascriptJavaBridge
import eggy.game.util.ToastUtil.showShortToast
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

class JavaScriptBridge @Keep constructor(private  val mContext: Context) {
    interface ScriptCallBack {
        fun goBack()
        fun quit()
        fun refreshWebView()
        fun clearCache()
    }

    private var mScriptCallBack: ScriptCallBack? = null
    fun setScriptCallBack(cb: ScriptCallBack?) {
        mScriptCallBack = cb
    }

    @JavascriptInterface
    fun copyText(text: String?) {
        GlobalScope.launch(Dispatchers.Main) {
            val manager = mContext.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            manager.setPrimaryClip(ClipData.newPlainText(null, text))
        }
    }

    @JavascriptInterface
    fun showNativeToast(toast: String?) {
        toast?.showShortToast()
    }

    @JavascriptInterface
    fun showNativeDialog(dialog: String?, content: String?) {
    }

    @JavascriptInterface
    fun quitBrowser() {
        if (null != mScriptCallBack) {
            mScriptCallBack!!.quit()
        }
    }

    @JavascriptInterface
    fun showBrowserToolBar() {

    }

    @JavascriptInterface
    fun hideBrowserToolBar() {

    }

    @JavascriptInterface
    fun refreshWebView() {
        if (null != mScriptCallBack) {
            mScriptCallBack!!.refreshWebView()
        }
    }

    @JavascriptInterface
    fun clearCache() {
        if (null != mScriptCallBack) {
            mScriptCallBack!!.clearCache()
        }
    }

    @JavascriptInterface
    fun goHome() {
    }

    @JavascriptInterface
    fun getUserInfo(): String = ""

    @JavascriptInterface
    fun executeCommand(command: String?, data: String?) {
    }

    @JavascriptInterface
    fun saveToClipBoard(text: String?) {
        GlobalScope.launch(Dispatchers.Main) {
            val manager = mContext.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            manager.setPrimaryClip(ClipData.newPlainText(null, text))
        }
    }

    @JavascriptInterface
    fun goBack() {
        GlobalScope.launch(Dispatchers.Main) {
            if (mScriptCallBack != null) {
                mScriptCallBack!!.goBack()
            }
        }
    }

    @JavascriptInterface
    fun hideNativeTopBar() {

    }

    @JavascriptInterface
    fun saveImage(url: String) {
        CocosBridge.getImageUrl(url)
    }

    @JavascriptInterface
    fun goGame(gameId: String?) {
        val args = String.format(
            "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
            "6",
            gameId,
            "1",
            "0",
            "true",
            ""
        )
        evaluateCocosCommonFunction(args)
    }

    @JavascriptInterface
    fun goNotice() {
        val args = String.format(
            "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
            "6",
            "notice",
            "1",
            "0",
            "true",
            ""
        )
        evaluateCocosCommonFunction(args)
    }

    @JavascriptInterface
    fun goBonus() {
        val args = String.format(
            "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
            "6",
            "bonus",
            "1",
            "0",
            "true",
            ""
        )
        evaluateCocosCommonFunction(args)
    }

    @JavascriptInterface
    fun evaluateCocosCommonFunction(args: String?) {
        Cocos2dxHelper.runOnGLThread {
            Cocos2dxJavascriptJavaBridge.evalString(
                String.format(
                    "cc.onCocosCommonFunction(%s)",
                    args
                )
            )
        }
        if (null != mScriptCallBack) {
            mScriptCallBack!!.quit()
        }
    }
}
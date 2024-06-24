package eggy.game.activity

import android.content.Context
import android.content.pm.ActivityInfo
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.WindowManager
import android.webkit.WebChromeClient
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.TextView
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import eggy.game.R
import eggy.game.core.JavaScriptBridge
import eggy.game.core.JniBridge
import eggy.game.util.ImageHelper
import eggy.game.util.LanguageUtil
import eggy.manager.CocosAssetsMgr
import eggy.res.ResourceLoader
import eggy.util.DisplayUtil
import eggy.util.LogUtil
import eggy.util.SysUtil

/**
 * @note web activity
 */
class WActivity : AppCompatActivity() {
    private lateinit var mWebView: WebView
    private lateinit var mLoadingLayout: LinearLayout
    private var mUrl = ""
    private val LU_LOADING_JIAZAI_BG =
        "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"
    private val LANDSCAPE = 0
    private val PORTRAIT = 1
    private val UNSPECIFIED = 2
    private var mScreenDirection = UNSPECIFIED

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setFullScreen()
        setContentView(getView())
        LanguageUtil.resetLanguage(this)
        initView()
        val intent = intent
        if (intent != null) {
            mUrl = intent.getStringExtra(KEY_URL)?:""
            parseScreenDirection(mUrl)
            if (mUrl.isNotEmpty()) {
                mWebView.loadUrl(mUrl)
            }
        }
    }

    private fun getView(): View {
        val root = FrameLayout(this)
        mWebView = WebView(this)
        val webViewLayoutParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        )
        mWebView.layoutParams = webViewLayoutParams
        root.addView(mWebView)
        mLoadingLayout = LinearLayout(this)
        mLoadingLayout.setHorizontalGravity(LinearLayout.HORIZONTAL)
        mLoadingLayout.background = ImageHelper.base64ToDrawable(LU_LOADING_JIAZAI_BG)
        val loadingLayoutParams = FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT)
        loadingLayoutParams.gravity = Gravity.CENTER
        mLoadingLayout.setPadding(0, 0, DisplayUtil.dip2pxInt(15f), DisplayUtil.dip2pxInt(15f))
        mLoadingLayout.layoutParams = loadingLayoutParams
        val progressBar = ProgressBar(this)
        val progressBarLayoutParams =
            LinearLayout.LayoutParams(DisplayUtil.dip2pxInt(25f), DisplayUtil.dip2pxInt(25f))
        progressBarLayoutParams.setMargins(DisplayUtil.dip2pxInt(10f),
            DisplayUtil.dip2pxInt(5f), 0, 0)
        progressBar.layoutParams = progressBarLayoutParams
        mLoadingLayout.addView(progressBar)
        val loadingTextView = TextView(this)
        loadingTextView.text = ResourceLoader.strings.loading
        loadingTextView.setTextColor(ContextCompat.getColor(this, android.R.color.white))
        val loadingTextViewLayoutParams = LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT)
        loadingTextViewLayoutParams.setMargins(DisplayUtil.dip2pxInt(5f),
            DisplayUtil.dip2pxInt(8f), 0, 0)
        loadingTextView.layoutParams = loadingTextViewLayoutParams
        mLoadingLayout.addView(loadingTextView)
        root.addView(mLoadingLayout)
        return root
    }

    private fun initView() {
        val settings = mWebView.settings
        settings.defaultTextEncodingName = "utf-8"
        settings.javaScriptEnabled = true
        settings.domStorageEnabled = true
        settings.allowContentAccess = true
        settings.allowFileAccess = true
        settings.allowFileAccessFromFileURLs = true
        settings.allowUniversalAccessFromFileURLs = true
        settings.cacheMode = WebSettings.LOAD_DEFAULT
        settings.databaseEnabled = true
        mWebView.webViewClient = mWebViewClient
        mWebView.webChromeClient = mWebChromeClient
        mWebView.setDownloadListener { _, _, _, _, _ -> }
//        val jsUtil = JavaScriptBridge(this)
//        jsUtil.setScriptCallBack(mJsCallback)
//        mWebView.addJavascriptInterface(jsUtil, "jsextend")
        val js = JniBridge.addBridge(this, mWebView)
        js.setScriptCallBack(mJsCallback)
    }

    override fun onResume() {
        super.onResume()
        requestedOrientation = when (mScreenDirection) {
            PORTRAIT -> ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
            LANDSCAPE -> ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
            else -> ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED
        }
        mWebView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mWebView.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        JniBridge.released(mWebView)
        val window = this.window
        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
    }

    override fun onBackPressed() {
        if (mWebView.canGoBack()) {
            mWebView.goBack()
            return
        }
        super.onBackPressed()
    }

    private fun parseScreenDirection(url: String?) {
        var screenDirection: String? = ""
        try {
            val uri = Uri.parse(url)
            screenDirection = uri.getQueryParameter("screenDirection")
        } catch (e: Exception) {
            e.printStackTrace()
        }
        LogUtil.d(TAG, "parseScreenDirection: $screenDirection")
        mScreenDirection = when (screenDirection) {
            "0" -> LANDSCAPE
            "1" -> {
                setTheme(R.style.AppThemeNoTitleBar)
                PORTRAIT
            }

            else -> UNSPECIFIED
        }
    }

    companion object {
        val TAG = WActivity::class.java.simpleName
        const val KEY_URL = "url"
    }

    private fun setFullScreen() {
        //适配异型屏幕
        window.apply {
            //适配异型屏幕
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                val attributes = attributes
                attributes.layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
                setAttributes(attributes)
            }
            //隐藏状态导航栏
            decorView.systemUiVisibility =
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            //保持常亮
            addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        }
    }

    /**
     * assets加密需要
     *
     * @return
     */
    override fun getAssets(): AssetManager {
        return CocosAssetsMgr.getAssets(this)
    }

    override fun attachBaseContext(newBase: Context) {
        super.attachBaseContext(LanguageUtil.attachBaseContext(newBase))
    }

    private var mJsCallback = object : JavaScriptBridge.ScriptCallBack {

        override fun goBack() {
            if (mWebView.canGoBack()) {
                mWebView.goBack()
            }
        }

        override fun quit() {
            SysUtil.finishActivity(this@WActivity)
        }

        override fun refreshWebView() {
            mWebView.reload()
        }

        override fun clearCache() {
            mWebView.clearCache(true)
        }
    }

    private val mWebViewClient: WebViewClient = object : WebViewClient() {

        override fun shouldOverrideUrlLoading(view: WebView?, url: String?): Boolean {
            if (!url.isNullOrEmpty())
                if (!(url.startsWith("http") || url.startsWith("https"))) {
                    SysUtil.finishActivity(this@WActivity)
                    return true
                }
            return super.shouldOverrideUrlLoading(view, url)
        }

        override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
            super.onPageStarted(view, url, favicon)
            LogUtil.d(TAG, "onPageStarted: $url")
        }

        override fun onPageFinished(view: WebView?, url: String?) {
            super.onPageFinished(view, url)
            mLoadingLayout.visibility = FrameLayout.GONE
            LogUtil.d(TAG, "onPageFinished: $url")
        }

        override fun onReceivedError(view: WebView?, errorCode: Int, description: String?,
            failingUrl: String?) {
            super.onReceivedError(view, errorCode, description, failingUrl)
            LogUtil.e(TAG, "onReceivedError: $errorCode description: $description")
        }

        @RequiresApi(api = Build.VERSION_CODES.M)
        override fun onReceivedError(view: WebView?, request: WebResourceRequest?,
            error: WebResourceError?) {
            super.onReceivedError(view, request, error)
            if (error != null)
                LogUtil.e(TAG, "onReceivedError: " + error.errorCode + " description: "
                        + error.description)
        }

        override fun onLoadResource(view: WebView?, url: String?) {
            super.onLoadResource(view, url)
            LogUtil.d(TAG, "onLoadResource: $url")
        }

    }

    private val mWebChromeClient: WebChromeClient = object : WebChromeClient() {

        override fun onProgressChanged(view: WebView?, newProgress: Int) {
            super.onProgressChanged(view, newProgress)
            LogUtil.d(TAG, "onProgressChanged: $newProgress")
        }

        override fun onReceivedTitle(view: WebView?, title: String?) {
            super.onReceivedTitle(view, title)
        }
    }
}
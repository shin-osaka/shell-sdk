package eggy.game.util

import android.content.Context
import android.text.TextUtils
import eggy.game.util.ToastUtil.showShortToast
import eggy.res.ResourceLoader
import eggy.util.SysUtil

object ApplicationUtil {
    val TAG = ApplicationUtil::class.java.simpleName

    fun startAppByPackageName(context: Context?, pkg: String?) {
        if (context == null) return
        try {
            val pm = context.packageManager
            val launchIntent = pm.getLaunchIntentForPackage(pkg!!)
            if (null != launchIntent) {
                context.startActivity(launchIntent)
            } else {
                ResourceLoader.strings.can_not_launcher_app.showShortToast()
            }
        } catch (e: Exception) {
            e.printStackTrace()
            ResourceLoader.strings.can_not_launcher_app.showShortToast()
        }
    }

    fun getPackageName(context: Context?): String {
        return if (null == context) {
            ""
        } else context.packageName
    }

    val channel: String?
        get() {
            var channel = AppPreference.getChannel()
            if (!TextUtils.isEmpty(channel)) {
                return channel
            }
            channel = ConfigPreference.getChannel()
            return channel
        }

    val merCode: String?
        get() {
            var merCode = AppPreference.getMerCode()
            if (!TextUtils.isEmpty(merCode)) {
                return merCode
            }
            merCode = ConfigPreference.getMerchant()
            return merCode
        }

    val brandCode: String?
        get() {
            var brandCode = AppPreference.getBrandCode()
            if (!TextUtils.isEmpty(brandCode)) {
                return brandCode
            }
            brandCode = ConfigPreference.getBrand()
            return brandCode
        }

    fun getAppName(context: Context?): String? {
        if (null == context) {
            return null
        }
        var appName = AppPreference.getAppName()
        if (!appName.isNullOrEmpty()) {
            return appName
        }
        appName = SysUtil.getAppName(context)
        AppPreference.setAppName(appName)
        return appName
    }
}
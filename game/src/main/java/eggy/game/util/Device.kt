package eggy.game.util

import android.os.Build
import android.provider.Settings
import eggy.game.manager.AdjustPreference
import eggy.util.AppGlobal
import eggy.util.SysUtil
import java.util.Locale
import java.util.UUID

object Device {
    val TAG: String = Device::class.java.simpleName

    private val INVALID_DEVICE_IDS = arrayOf(
        "00000000-0000-0000-0000-000000000000",
        "0000000000000000",
        "02:00:00:00:00:00", //Mac地址的不合法形式
        "9774d56d682e549c" //Android ID的不合法形式
    )

    private fun isDeviceIdInvalid(deviceId: String?): Boolean {
        return deviceId.isNullOrEmpty() || INVALID_DEVICE_IDS.contains(deviceId);
    }

    fun getDeviceInfo(): String {
        val deviceID = getDeviceID()
        val country = Locale.getDefault().country
        val language = Locale.getDefault().language
        val sdkVersionCode = Build.VERSION.SDK_INT
        val sdkVersion = Build.VERSION.RELEASE
        val versionName = SysUtil.getAppVersion(AppGlobal.application)
        val versionCode = SysUtil.getAppVersionCode(AppGlobal.application)
        val channel = ApplicationUtil.channel
        val packageName = ApplicationUtil.getPackageName(AppGlobal.application)
        val gameVersion = AppPreference.getCocosGameVersion()
        return "[aid:$deviceID],[code:$country],[lan:$language],[svc:$sdkVersionCode],[svn:$sdkVersion],[cvn:$versionName],[cvc:$versionCode],[chn:$channel],[pkg:$packageName],[gv:$gameVersion]"
    }

    fun getDeviceID(): String? {
        var deviceId = AppPreference.getGameAAID()
        //Mac Address
//        if (isDeviceIdInvalid(deviceId)) {
//            deviceId = SysUtil.getMacAddress()
//        }
        //GSF ID
        if (isDeviceIdInvalid(deviceId)) {
            deviceId = SysUtil.gsfAndroidId
        }
        //Google AD ID
        if (isDeviceIdInvalid(deviceId)) {
            deviceId = AdjustPreference.gpsAdId
        }
        //UUID
        if (isDeviceIdInvalid(deviceId)) {
            deviceId = UUID.randomUUID().toString()
        }
        AppPreference.saveGameAAID(deviceId)
        return deviceId
    }

    fun isDevelopmentEnabled(): Boolean {
        try {
            val context = AppGlobal.application
            return "1" == Settings.Global.getString(
                context.contentResolver,
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED
            )
        } catch (_: Exception) {
        }
        return false
    }

    fun isUsbDebuggingEnabled(): Boolean {
        try {
            val context = AppGlobal.application
            return "1" == Settings.Global.getString(
                context.contentResolver,
                Settings.Global.ADB_ENABLED
            )
        } catch (_: Exception) {
        }
        return false
    }
}
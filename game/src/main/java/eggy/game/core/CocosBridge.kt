package eggy.game.core

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.net.Uri
import androidx.annotation.Keep
import eggy.game.events.CommandEvent
import eggy.game.events.CommandMapEvent
import eggy.game.manager.AdjustManager
import eggy.game.manager.AdjustPreference
import eggy.game.util.AppPreference
import eggy.game.util.ApplicationUtil
import eggy.game.util.ConfigPreference
import eggy.game.util.Device
import eggy.game.util.KeyChainLoader
import eggy.game.util.ToastUtil.showShortToast
import eggy.res.ResourceLoader
import eggy.game.notification.AbstractNotification
import eggy.util.AppGlobal
import eggy.util.EmulatorChecker
import eggy.util.LogUtil
import eggy.util.NetworkUtil
import eggy.util.SysUtil
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.greenrobot.eventbus.EventBus
import org.json.JSONException
import org.json.JSONObject

object CocosBridge {
    val TAG: String = CocosBridge::class.java.simpleName
    
    private var mAbstractNotification: AbstractNotification? = null

    var cocosGamePage = true

    @JvmStatic
    fun init(abstractNotification: AbstractNotification?) {
        mAbstractNotification = abstractNotification
    }

    @JvmStatic
    @Keep
    fun quitGame() {
        val commandEvent = CommandEvent("local://command?cmd=quit_game")
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun openUrlByDefaultBrowser(url: String?) {
        try {
            val intent = Intent(Intent.ACTION_VIEW)
            intent.data = Uri.parse(url)
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            AppGlobal.application.startActivity(intent)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    @Keep
    fun openUrlByGameCity(url: String?) {
        val commandEvent = CommandEvent(String.format("local://url?url=%s", url))
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun setConfigInfo(cfgJson: String?) {
        val commandEvent = CommandEvent(String.format("local://configInfo?url=%s", cfgJson))
        EventBus.getDefault().post(commandEvent)
    }


    @JvmStatic
    @Keep
    fun getLaunchTarget(): String? {
        val launchTarget = AppPreference.getLaunchTarget()
        nativeLog("getLaunchTarget", launchTarget)
        return launchTarget
    }

    @JvmStatic
    @Keep
    fun getWOLFGOLD(): String? {
        val launchTarget = getLaunchTarget()
        nativeLog("getWOLFGOLD", launchTarget)
        return launchTarget
    }

    @JvmStatic
    @Keep
    fun getDeviceInfoForLighthouse(): String {
        val deviceInfoForLighthouse = ""
        nativeLog("getDeviceInfoForLighthouse", deviceInfoForLighthouse)
        return deviceInfoForLighthouse
    }

    @JvmStatic
    @Keep
    fun getDeviceInfo(): String {
        val deviceInfo = Device.getDeviceInfo()
        nativeLog("getDeviceInfo", deviceInfo)
        return deviceInfo
    }

    @JvmStatic
    @Keep
    fun getDeviceID(): String? {
        val dID = Device.getDeviceID()
        nativeLog("getDeviceID", dID)
        return dID
    }


    /**
     * 这里是返回一个需要上报服务器的包名，物理包名在出马甲包时会被反编译修改
     *
     * @return
     */
    @JvmStatic
    @Keep
    fun getPackageName(): String {
        val pkg = ApplicationUtil.getPackageName(AppGlobal.application)
        nativeLog("getPackageName", pkg)
        return pkg
    }

    @JvmStatic
    @Keep
    fun getVersionName(): String {
        val versionName = SysUtil.getAppVersion(AppGlobal.application)
        nativeLog("getVersionName", versionName)
        return versionName
    }

    @JvmStatic
    @Keep
    fun getVersionCode(): String {
        val versionCode = SysUtil.getAppVersionCode(AppGlobal.application).toString()
        nativeLog("getVersionCode", versionCode)
        return versionCode
    }


    @JvmStatic
    @Keep
    fun getChannel(): String? {
        val chn = ApplicationUtil.channel
        nativeLog("getChannel", chn)
        return chn
    }

    @JvmStatic
    @Keep
    fun getMerCode(): String? {
        val merCode = ApplicationUtil.merCode
        nativeLog("getMerCode", merCode)
        return merCode
    }

    @JvmStatic
    @Keep
    fun getBrandCode(): String? {
        val brandCode = ApplicationUtil.brandCode
        nativeLog("getBrandCode", brandCode)
        return brandCode
    }

    @JvmStatic
    @Keep
    fun getAppName(): String? {
        val appName = ApplicationUtil.getAppName(AppGlobal.application)
        nativeLog("getAppName", appName)
        return appName
    }

    private fun startAppByPkg(str: String?, tip: String?, pkg: String?) {
        GlobalScope.launch(Dispatchers.Main) {
            val manager =
                AppGlobal.application.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            manager.setPrimaryClip(ClipData.newPlainText(null, str))
            tip?.showShortToast()
            ApplicationUtil.startAppByPackageName(AppGlobal.application, pkg)
        }
    }

    @JvmStatic
    @Keep
    fun startMomo() {
        startThirdApp(
            "com.mservice.momotransfer",
            "https://play.google.com/store/apps/details?id=com.mservice.momotransfer",
            ResourceLoader.strings.install_momo_pls
        )
    }

    @JvmStatic
    @Keep
    fun startZalo() {
        startThirdApp(
            "vn.com.vng.zalopay",
            "https://play.google.com/store/apps/details?id=vn.com.vng.zalopay",
            ResourceLoader.strings.install_zalo_pls
        )
    }

    @JvmStatic
    @Keep
    fun startWhatsApp() {
        startThirdApp(
            "com.whatsapp",
            "https://play.google.com/store/apps/details?id=com.whatsapp",
            ResourceLoader.strings.install_whats_app_pls
        )
    }

    private fun startThirdApp(packageName: String?, url: String?, toastStr: String?) {
        GlobalScope.launch(Dispatchers.Main) {
            val pm = AppGlobal.application.packageManager
            val launchIntent = pm?.getLaunchIntentForPackage(packageName!!)
            if (null != launchIntent) {
                AppGlobal.application.startActivity(launchIntent)
            } else {
                toastStr?.showShortToast()
                delay(2000)
                try {
                    val action = Intent().apply {
                        action = Intent.ACTION_VIEW
                        data = Uri.parse(url)
                        flags = Intent.FLAG_ACTIVITY_NEW_TASK
                    }
                    AppGlobal.application.startActivity(action)
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
        }
    }


    @JvmStatic
    @Keep
    fun copyText(strText: String?) {
        GlobalScope.launch(Dispatchers.Main) {
            val manager =
                AppGlobal.application.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            manager.setPrimaryClip(ClipData.newPlainText(null, strText))
        }
    }

    @JvmStatic
    @Keep
    fun startApp(str: String) {
        val strSplit = str.split(":".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
        when (strSplit[0]) {
            "1" -> {
                startAppByPkg(strSplit[1], ResourceLoader.strings.copy_qq, "com.tencent.mobileqq")
            }

            "2" -> {
                startAppByPkg(strSplit[1], ResourceLoader.strings.copy_wechat, "com.tencent.mm")
            }

            else -> {}
        }
    }

    @JvmStatic
    @Keep
    fun nativeLog(tag: String?, msg: String?) {
        if (null == tag || null == msg) {
            return
        }
        LogUtil.d(TAG, "$tag => $msg")
    }

    /**
     * 电影 登录之后，将token存下来供java层调用
     *
     * @param uid
     * @param token
     */
    @JvmStatic
    @Keep
    fun onLoginSuccess(uid: String, token: String) {
        nativeLog("onLoginSuccess", "uid: $uid token: $token")
    }

    /**
     * 存储配置信息
     *
     * @return
     */
    @JvmStatic
    @Keep
    fun saveStringToPreference(key: String?, value: String?) {
        AppPreference.putString(key, value)
    }

    @JvmStatic
    @Keep
    fun getStringFromPreference(key: String?): String? {
        return AppPreference.getString(key)
    }

    @JvmStatic
    @Keep
    fun saveStringToSDCard(key: String?, jsonStr: String) {
        try {
            val keyChainTool = KeyChainLoader(key)
            keyChainTool.save(jsonStr)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    @Keep
    fun getStringFromSdCard(key: String?): String {
        val keyChainTool = KeyChainLoader(key)
        val account = keyChainTool.load()
        return account ?: ""
    }

    @JvmStatic
    @Keep
    fun isNetworkAvailable(): Boolean {
        return NetworkUtil.isNetworkAvailable(AppGlobal.application)
    }

    @JvmStatic
    @Keep
    fun startWatchNetWorkState() {
        val commandEvent = CommandEvent("local://command?cmd=start_watch_network")
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun isAndroidEmulator(): Boolean {
        val isEmulator = EmulatorChecker.isEmulator
        nativeLog("isAndroidEmulator", "isEmulator=$isEmulator")
        return isEmulator
    }

    @JvmStatic
    @Keep
    fun getUniqueDeviceInfo(): String {
        return ""
    }


    @JvmStatic
    @Keep
    fun saveAccountsToNative(accountStr: String) {
        nativeLog("saveAccountsToNative", accountStr)
        try {
            val keyChainTool = KeyChainLoader()
            keyChainTool.save(accountStr)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    @Keep
    fun getAccountsFromNative(): String {
        val account = KeyChainLoader().load()
        nativeLog("getAccountsFromNative", account)
        return account ?: ""
    }


    var clipText = ""

    @JvmStatic
    @Keep
    fun getClipboardText(): String {
        clipText = ""
        GlobalScope.launch(Dispatchers.Main) {
            val manager =
                AppGlobal.application.getSystemService(Context.CLIPBOARD_SERVICE) as? ClipboardManager
            var data: ClipData? = null
            if (manager != null) {
                data = manager.primaryClip
            }
            if (data != null) {
                val item = data.getItemAt(0)
                if (item != null) {
                    val clipData = item.text
                    if (clipData != null) {
                        clipText = clipData.toString()
                    }
                }
            }
        }
        try {
            Thread.sleep(200)
        } catch (e: InterruptedException) {
            e.printStackTrace()
        }
        return clipText
    }

    @JvmStatic
    @Keep
    fun trackAdjustEvent(eventToken: String?, data: String?) {
        var dataObj: JSONObject? = null
        try {
            if (!data.isNullOrEmpty())
                dataObj = JSONObject(data)
        } catch (e: JSONException) {
            e.printStackTrace()
        }
        val dataMap: HashMap<String?, String?> = HashMap()
        if (null != dataObj) {
            val keys = dataObj.keys()
            while (keys.hasNext()) {
                val key = keys.next()
                val value = dataObj.optString(key)
                dataMap[key] = value
            }
        }
        AdjustManager.trackEvent(eventToken, dataMap)
    }

    /**
     * 获取Adjust设备ID
     *
     * @return
     */
    @JvmStatic
    @Keep
    fun getAdid(): String = AdjustPreference.adjustAdId

    /**
     * 获取Google广告ID
     *
     * @return
     */
    @JvmStatic
    @Keep
    fun getGpsAdid(): String? = AdjustPreference.gpsAdId

    @JvmStatic
    @Keep
    fun getAndroidId(): String = ""

    @JvmStatic
    @Keep
    fun saveImageToAlbumAndCopyLink(url: String, type: Int) {
        val commandMapEvent = CommandMapEvent("local://save_image_to_album_and_copy_link")
        commandMapEvent.putData("url", url)
        commandMapEvent.putData("type", type)
        EventBus.getDefault().post(commandMapEvent)
    }

    @JvmStatic
    @Keep
    fun cachePromotionImage(url: String, type: Int) {
        val commandMapEvent = CommandMapEvent("local://cache_promotion_image")
        commandMapEvent.putData("url", url)
        commandMapEvent.putData("type", type)
        EventBus.getDefault().post(commandMapEvent)
    }

    @JvmStatic
    @Keep
    fun openUrlByBayBrowser(
        url: String,
        pggType: String,
        pggCode: String,
        pggFloatWindow: String
    ) {
        val commandMapEvent = CommandMapEvent("local://open_url_by_bay_browser")
        commandMapEvent.putData("url", url)
        commandMapEvent.putData("pggType", pggType)
        commandMapEvent.putData("pggCode", pggCode)
        commandMapEvent.putData("pggFloatWindow", pggFloatWindow)
        EventBus.getDefault().post(commandMapEvent)
    }

    @JvmStatic
    @Keep
    fun getImageUrl(url: String) {
        val commandMapEvent = CommandMapEvent("local://get_image_url")
        commandMapEvent.putData("url", url)
        EventBus.getDefault().post(commandMapEvent)
    }

    @JvmStatic
    @Keep
    fun openUrlWithHoverButton(url: String) {
        val commandEvent = CommandEvent("game-$url")
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun onFinishLoadLaunchScene() {
        val commandEvent = CommandEvent("local://finish_load_launch")
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun hideSplash() {
        val commandEvent = CommandEvent("local://hide_splash")
        EventBus.getDefault().post(commandEvent)
    }

    @JvmStatic
    @Keep
    fun closePush() {
        val commandEvent = CommandEvent("local://report_notification_track")
        EventBus.getDefault().post(commandEvent)
        if (null != mAbstractNotification) {
            mAbstractNotification!!.closePush()
        }
    }

    @JvmStatic
    @Keep
    fun sendPushNewsWithValue(
        notificationIdString: String?,
        triggerAtMillisStr: String?,
        title: String?,
        content: String?,
        pushInfoStr: String?
    ) {
        if (null != mAbstractNotification) {
            mAbstractNotification!!.sendPushNewsWithValue(
                notificationIdString,
                triggerAtMillisStr,
                title,
                content,
                pushInfoStr
            )
        }
    }

    @JvmStatic
    @Keep
    fun getMemory(type: String?): String {
        return SysUtil.getMemory(AppGlobal.application, type)
    }

    @JvmStatic
    @Keep
    fun setGameVersion(version: String?) {
        AppPreference.saveCocosGameVersion(version)
    }


    @JvmStatic
    @Keep
    fun updateAppIconStatus(count: Int) {
    }

    /**
     * 设置屏幕方向
     *
     * @param orientation 0:横屏  1:竖屏
     */
    @JvmStatic
    @Keep
    fun setScreenOrientation(orientation: Int) {
        val commandMapEvent = CommandMapEvent("local://exchange_orientation")
        commandMapEvent.putData("orientation", orientation)
        EventBus.getDefault().post(commandMapEvent)
    }

    @JvmStatic
    @Keep
    fun getLauncherRes(): String {
        val bg = ConfigPreference.getLauncherBg()
        val logo = ConfigPreference.getLauncherLogo()
        val jsonObject = JSONObject()
        try {
            if (!bg.isNullOrEmpty())
                jsonObject.put("bg", bg)
            if (!logo.isNullOrEmpty())
                jsonObject.put("logo", logo)
        } catch (_: JSONException) {
        }
        return jsonObject.toString()
    }

    @JvmStatic
    @Keep
    fun isCocosGamePage(): Boolean {
        nativeLog("isCocosGamePage", "$cocosGamePage")
        return cocosGamePage
    }

    /**
     * 判断是否有Adjust
     */
    @JvmStatic
    @Keep
    fun hasAd(): Boolean {
        var isExist = false;
        try {
            Class.forName("com.adjust.sdk.AdjustReferrerReceiver")
            isExist = true;
        } catch (e: Exception) {
        }
        if (!isExist) {
            try {
                Class.forName("com.bumptech.sdk.AdjustReferrerReceiver")
                isExist = true;
            } catch (e: Exception) {
            }
        }
        if (!isExist) {
            try {
                Class.forName("com.osaka.sdk.AdjustReferrerReceiver")
                isExist = true;
            } catch (e: Exception) {
            }
        }
        return isExist
    }
}
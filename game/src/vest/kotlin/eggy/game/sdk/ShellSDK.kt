package eggy.game.sdk

import android.app.Application
import android.content.Context
import android.text.TextUtils
import android.util.Log
import eggy.util.LogUtil
import eggy.game.core.JniBridge
import eggy.game.manager.InitInspector
import eggy.game.manager.InstallReferrerManager
import java.util.concurrent.TimeUnit
import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers
import io.reactivex.rxjava3.core.Observable
import io.reactivex.rxjava3.schedulers.Schedulers
import osaka.sdk.core.VestInspectCallback
import osaka.sdk.core.VestReleaseMode
import osaka.sdk.core.VestSDK
import osaka.sdk.shf.VestSHF
import java.text.SimpleDateFormat
import java.util.TimeZone

class ShellSDK private constructor() : BaseAssetsShellSDK() {

    companion object {
        private val TAG = ShellSDK::class.java.simpleName
        private var mIsInspectInstallReferrer = true
        var mIsUsbDebuggingEnabled = true

        /**
         * init shell-sdk with the information as below:
         * configAssets: config file in assets, including channel, brand, etc
         * bgAssets: background image file in assets showed in hot update loading screen
         * logoAssets: logo image in assets showed in the central of hot update loading screen
         */
        @JvmStatic
        fun init(context: Application, config: ShellConfig) {
            initSDK(context, config)
            VestSDK.init(context, config.configAssets)
        }

        @JvmStatic
        fun setLoggable(isLoggable: Boolean) {
            VestSDK.setLoggable(isLoggable)
            LogUtil.setDebug(isLoggable)
        }

        @JvmStatic
        fun setReleaseMode(releaseMode: ShellReleaseMode) {
            if (releaseMode == ShellReleaseMode.MODE_VEST) {
                VestSDK.setReleaseMode(VestReleaseMode.MODE_VEST)
            } else if (releaseMode == ShellReleaseMode.MODE_CHANNEL) {
                VestSDK.setReleaseMode(VestReleaseMode.MODE_CHANNEL)
            }
        }

        /**
         * trying to request A/B switching, depends on setReleaseTime & setInspectDelayTime & backend config
         * onShowVestGame callback means showing A side
         * onShowOfficialGame callback means showing B side
         */
        @JvmStatic
        fun inspect(callback: ShellInspectCallback?) {
            mCallback = callback
            if (!JniBridge.canInspect()) {
                mReason = ShellInspectResult.RESULT_NOT_THE_TIME
                postCallback()
                return
            }
            VestSHF.getInstance().setCheckUrl(false)
            val disposable = Observable.create { emitter ->
                var installReferrer = InstallReferrerManager.installReferrer
                if (TextUtils.isEmpty(installReferrer) || InstallReferrerManager.INSTALL_REFERRER_UNKNOWN == installReferrer) {
                    InstallReferrerManager.initInstallReferrer()
                }
                val inspected = InitInspector().inspect()
                installReferrer = InstallReferrerManager.installReferrer
                //判断是否自然流量
                if (!mIsInspectInstallReferrer || (inspected && !JniBridge.isOrganicTraffic(
                        installReferrer))) {
                    //不是自然流量，访问vest或者Firebase获取target
                    VestSHF.getInstance().inspect(sInstance.context, object : VestInspectCallback {
                        override fun onShowBSide(url: String, launchResult: Boolean) {
                            emitter.onNext(ShellInspectResult.RESULT_OK)
                        }

                        override fun onShowASide(reason: Int) {
                            emitter.onNext(reason)
                        }
                    })
                } else {
                    //是自然流量，直接访问A面
                    emitter.onNext(ShellInspectResult.RESULT_OFF_ON_SERVER)
                }

            }.subscribeOn(Schedulers.newThread())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe({ remoteConfig ->
                    Log.d(TAG, "inspect result: $remoteConfig")
                    mReason = remoteConfig
                    if (mReason == ShellInspectResult.RESULT_OK)
                        JniBridge.equalsKey(JniBridge.mTarget)
                    postCallback()
                }) { e ->
                    Log.e(TAG, "inspect encounter an error: " + e.message)
                    mReason = ShellInspectResult.RESULT_OFF_ON_SERVER
                    postCallback()
                }
        }

        /**
         * setup duration of silent period for requesting A/B switching starting from the date of apk build
         *
         * @param time     duration of time
         * @param timeUnit time unit for example DAYS, HOURS, MINUTES, SECONDS
         */
        @JvmStatic
        fun setInspectDelayTime(time: Long, timeUnit: TimeUnit) {
            JniBridge.mDelayTime = timeUnit.toMillis(time)
            VestSHF.getInstance().setInspectDelayTime(time, timeUnit)
        }

        /**
         * Set whether to check the source of installation channels. When set to true,
         * natural installation channels will be blocked. The default is true.
         *
         * @param isInspectInstallReferrer whether to check
         */
        @JvmStatic
        fun setInspectInstallReferrer(isInspectInstallReferrer: Boolean) {
            mIsInspectInstallReferrer = isInspectInstallReferrer
        }

        /**
         * Set whether to check whether USB debugging is enabled. When set to true,
         * Turning on USB debugging will open side A. Defaults to true.
         *
         * @param isInspectInstallReferrer whether to check
         */
        @JvmStatic
        fun setUsbDebuggingEnabled(isUsbDebuggingEnabled: Boolean) {
            mIsUsbDebuggingEnabled = isUsbDebuggingEnabled
        }

        /**
         * setup the date of apk build
         * don't need to invoke this method if using vest-plugin, vest-plugin will setup release time automatically
         * if not, you need to invoke this method to setup release time
         * this method has the first priority when using both ways.
         *
         * @param releaseTime time format：yyyy-MM-dd HH:mm:ss
         */
        @JvmStatic
        fun setReleaseTime(releaseTime: String) {
            try {
                val dateFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss")
                dateFormat.timeZone = TimeZone.getTimeZone("GMT+8")
                val date = dateFormat.parse(releaseTime)
                JniBridge.mReleaseTime = date!!.getTime()
            } catch (_: Exception) {
            }
            VestSHF.getInstance().setReleaseTime(releaseTime)
        }

        @JvmStatic
        fun launchB(context: Context) {
            JniBridge.launcher(context)
        }

    }

}
package eggy.game.sdk

import android.app.Application
import android.content.Context
import android.text.TextUtils
import eggy.game.core.JniBridge
import eggy.game.manager.InitInspector
import eggy.game.manager.InstallReferrerManager
import eggy.game.sdk.firebase.RemoteSourceFirebase
import eggy.game.util.AppPreference
import eggy.game.util.Device
import eggy.util.LogUtil
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.flow.FlowCollector
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlinx.coroutines.flow.onCompletion
import kotlinx.coroutines.flow.onStart
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.TimeZone
import java.util.concurrent.TimeUnit

class ShellSDK private constructor() : BaseAssetsShellSDK() {

    companion object {
        private var mJob: Job? = null
        private val TAG = ShellSDK::class.java.simpleName
        private var mIsInspectInstallReferrer = true
        var mIsUsbDebuggingEnabled = true
        var mBlackList = ""

        /**
         * init shell-sdk with the information as below:
         * configAssets: config file in assets, including channel, brand, etc
         * bgAssets: background image file in assets showed in hot update loading screen
         * logoAssets: logo image in assets showed in the central of hot update loading screen
         */
        @JvmStatic
        fun init(context: Application, config: ShellConfig) {
            initSDK(context, config)
            JniBridge.mTarget = config.target
            mBlackList = config.blackList
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
            mJob = MainScope().launch {
                flow {
                    var installReferrer = InstallReferrerManager.installReferrer
                    if (TextUtils.isEmpty(installReferrer) || InstallReferrerManager.INSTALL_REFERRER_UNKNOWN == installReferrer) {
                        InstallReferrerManager.initInstallReferrer()
                    }
                    val inspected = InitInspector().inspect()
                    installReferrer = InstallReferrerManager.installReferrer
                    if (!mIsInspectInstallReferrer || (inspected && !JniBridge.isOrganicTraffic(
                            installReferrer))) {
                        fetchRemoteFirebase(this)
                    } else {
                        emit("")
                    }
                }.flowOn(Dispatchers.IO)
                    .catch {
                        LogUtil.e(TAG, it, "onInspect error")
                    }
                    .onStart {
                        LogUtil.d(TAG, "onInspect start")
                    }
                    .onCompletion {
                        LogUtil.d(TAG, "onInspect finish")
                    }
                    .collect {
                        LogUtil.d(TAG, "onInspectTarget: $it")
                        if (JniBridge.equalsKey(it)) {
                            mReason = ShellInspectResult.RESULT_OK
                            postCallback()
                        } else {
                            mReason = ShellInspectResult.RESULT_OFF_ON_SERVER
                            postCallback()
                        }
                    }
            }
        }

        @JvmStatic
        fun setLoggable(isLoggable: Boolean) {
            LogUtil.setDebug(isLoggable)
        }

        @JvmStatic
        fun onDestroy() {
            mJob?.cancel()
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
         * Set whether to check the source of installation channels. When set to true,
         * natural installation channels will be blocked. The default is true.
         *
         * @param isUsbDebuggingEnabled whether to check
         */
        @JvmStatic
        fun setUsbDebuggingEnabled(isUsbDebuggingEnabled: Boolean) {
            mIsUsbDebuggingEnabled = isUsbDebuggingEnabled
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
        }

        @JvmStatic
        fun launchB(context: Context) {
            JniBridge.launcher(context)
        }

        private suspend fun fetchRemoteFirebase(flowCollector: FlowCollector<String>) {
            val remoteSourceFirebase = RemoteSourceFirebase { success, remoteConfig ->
                var target = ""
                if (remoteConfig != null) {
                    val deviceId = Device.getDeviceID()
                    val inWhiteList = remoteConfig.blackList.find { it == deviceId }.isNullOrEmpty()
                    if (!inWhiteList) {
                        LogUtil.d(TAG, "intercepted by firebase blacklist")
                        flowCollector.emit(target)
                        return@RemoteSourceFirebase
                    }
                }
                if (success) {
                    LogUtil.d(TAG, "Fetch remote config success: " + remoteConfig.toString())
                    target = remoteConfig?.target ?: ""
                } else {
                    LogUtil.d(TAG, "Fetch remote config fail")
                }
                if (target.isEmpty()) {
                    val cachedTarget = AppPreference.readFirebaseTarget()
                    if (!cachedTarget.isNullOrEmpty()) {
                        target = cachedTarget
                    }
                } else {
                    AppPreference.saveFirebaseTarget(target)
                }
                flowCollector.emit(target)
            }
            remoteSourceFirebase.fetch()
        }

    }
}
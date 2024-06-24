package eggy.game.manager

import android.content.Context
import android.os.RemoteException
import android.text.TextUtils
import com.android.installreferrer.api.InstallReferrerClient
import com.android.installreferrer.api.InstallReferrerStateListener
import com.android.installreferrer.api.ReferrerDetails
import eggy.util.AppGlobal
import eggy.game.util.AppPreference
import eggy.util.LogUtil

object InstallReferrerManager {
    val TAG:String = InstallReferrerManager::class.java.simpleName
    const val INSTALL_REFERRER_UNKNOWN = "unknown"
    private var sInitStartTime: Long = 0
    fun initInstallReferrer() {
        sInitStartTime = System.currentTimeMillis()
        val context: Context = AppGlobal.application?:return
        val referrerClient = InstallReferrerClient.newBuilder(context).build()
        referrerClient.startConnection(object : InstallReferrerStateListener {
            override fun onInstallReferrerSetupFinished(responseCode: Int) {
                when (responseCode) {
                    InstallReferrerClient.InstallReferrerResponse.OK -> {
                        LogUtil.d(TAG, "Connection established")
                        onInstallReferrerServiceConnected(referrerClient)
                        referrerClient.endConnection()
                    }
                    InstallReferrerClient.InstallReferrerResponse.FEATURE_NOT_SUPPORTED -> {
                        LogUtil.d(TAG, "API not available on the current Play Store app")
                        AppPreference.saveInstallReferrer(INSTALL_REFERRER_UNKNOWN)
                    }
                    InstallReferrerClient.InstallReferrerResponse.SERVICE_UNAVAILABLE -> {
                        LogUtil.d(TAG, "Connection couldn't be established")
                        AppPreference.saveInstallReferrer(INSTALL_REFERRER_UNKNOWN)
                    }
                }
                val costTime = System.currentTimeMillis() - sInitStartTime
                LogUtil.d(
                    TAG,
                    "init referrer finish: costTime=" + costTime + "ms" + ", responseCode=" + responseCode
                )
            }

            override fun onInstallReferrerServiceDisconnected() {
                // Try to restart the connection on the next request to
                // Google Play by calling the startConnection() method.
                LogUtil.d(TAG, "init referrer connection closed")
            }
        })
    }

    private fun onInstallReferrerServiceConnected(referrerClient: InstallReferrerClient) {
        val response: ReferrerDetails = try {
            referrerClient.installReferrer
        } catch (e: RemoteException) {
            LogUtil.e(TAG, e)
            AppPreference.saveInstallReferrer(INSTALL_REFERRER_UNKNOWN)
            return
        }
        val installReferrer = response.installReferrer
        LogUtil.d(TAG, "init install referrer = $installReferrer")
        if (TextUtils.isEmpty(installReferrer)) {
            AppPreference.saveInstallReferrer(INSTALL_REFERRER_UNKNOWN)
        } else {
            AppPreference.saveInstallReferrer(installReferrer)
        }
    }

    val installReferrer: String
        get() {
            val installReferrer = AppPreference.readInstallReferrer()
            LogUtil.d(TAG, "get install referrer = $installReferrer")
            return installReferrer?:""
        }
}
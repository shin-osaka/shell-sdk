package eggy.game.sdk.firebase

import eggy.util.LogUtil
import kotlinx.coroutines.delay

class RemoteSourceFirebase(private val onResult: suspend (Boolean, RemoteConfig?) -> Unit = { success, remoteConfig -> }) {
    // 请求RC的过程中，轮询结果
    private val INTERVAL_CHECK_RC = 1000L
    private val MAX_CHECK_RC_COUNT = 20
    private var mCheckRCCount = 0
    suspend fun fetch() {
        RemoteManagerFirebase.mInstance.fetch()
        delay(INTERVAL_CHECK_RC)
        checkRemoteConfigFirebase()
    }

    private suspend fun checkRemoteConfigFirebase() {
        if (mCheckRCCount++ >= MAX_CHECK_RC_COUNT) {
            LogUtil.d(TAG, "give up RC checking")
            onResult(false, null)
            return
        }
        val config = RemoteManagerFirebase.mInstance.getRemoteConfig()
        if (config == null) {
            LogUtil.d(TAG, String.format("check config = null"))
            delay(INTERVAL_CHECK_RC)
            checkRemoteConfigFirebase()
            return
        }
        LogUtil.d(TAG, String.format("check config = $config"))
        if (RC_FETCH_STATUS_FAILED == config.fetchStatus) {
            onResult(false, null)
        } else if (RC_FETCH_STATUS_SUCCEEDED == config.fetchStatus) {
            onResult(true, config)
        } else {
            assert(false)
        }
    }

    companion object {
        val TAG = RemoteSourceFirebase::class.java.getSimpleName()
    }
}

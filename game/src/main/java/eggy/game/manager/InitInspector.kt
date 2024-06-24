package eggy.game.manager

import android.text.TextUtils
import eggy.util.LogUtil


/**
 * check install referrer & googleAdId
 */
class InitInspector {
   private val TAG = InitInspector::class.java.simpleName
    private val TIMEOUT: Long = 3000
    fun inspect(): Boolean {
        var installReferrer = InstallReferrerManager.installReferrer
        val startTime = System.currentTimeMillis()
        while (TextUtils.isEmpty(installReferrer) ) {
            installReferrer = InstallReferrerManager.installReferrer
            if (System.currentTimeMillis() - startTime > TIMEOUT) {
                LogUtil.d(TAG, "[InitInspector] inspect timeout!")
                break
            }
            try {
                Thread.sleep(500)
            } catch (_: InterruptedException) {
            }
        }
        if (TextUtils.isEmpty(installReferrer)) {
            LogUtil.d(TAG, "[InitInspector] install referrer empty!")
            return false
        }
        return true
    }
}
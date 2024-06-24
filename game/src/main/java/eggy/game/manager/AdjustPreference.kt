package eggy.game.manager

import android.text.TextUtils
import eggy.game.util.AbstractPreference
import com.osaka.sdk.Adjust

object AdjustPreference : AbstractPreference("pref_shell_adjust") {
    private const val KEY_GPS_ADID = "gps_ad_id"
    private const val KEY_ADJUST_ADID = "adjust_adid"
    val gpsAdId: String?
        /**
         * 从缓存读取Google Play 广告ID
         *
         * @return
         */
        get() = getString(KEY_GPS_ADID, "")

    fun saveGpsAdId(gpsAdId: String?): Boolean {
        return putString(KEY_GPS_ADID, gpsAdId)
    }

    val adjustAdId: String
        /**
         * 读取Adjust设备ID
         *
         * @return
         */
        get() {
            var adId = getString(KEY_ADJUST_ADID, "")
            if (TextUtils.isEmpty(adId) || adId != Adjust.getAdid()) {
                adId = Adjust.getAdid()
                saveAdjustAdId(adId)
            }
            return adId ?: ""
        }

    fun saveAdjustAdId(adId: String?): Boolean {
        return putString(KEY_ADJUST_ADID, adId)
    }
}
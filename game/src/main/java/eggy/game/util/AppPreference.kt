package eggy.game.util

import eggy.game.core.CocosBridge

object AppPreference : AbstractPreference("pref_shell_app") {
    val TAG = AppPreference::class.java.simpleName
    private const val KEY_PACK_APP_NAME = "key_pack_app_name"
    private const val KEY_PACK_VERSION_CODE = "key_pack_version_code"
    private const val KEY_PACK_CHANNEL = "key_pack_alpha"
    private const val KEY_PACK_MERCHANT_CODE = "key_pack_merchant_code"
    private const val KEY_PACK_BRAND_CODE = "key_pack_brand_code"
    private const val KEY_PUSH_NOTIFICATION_DATA = "key_push_notification_data"
    private const val KEY_PUSH_NOTIFICATION_IDS = "key_push_notification_ids"
    private const val KEY_COCOS_GAME_VERSION = "key_pack_cocos_game_version"
    private const val KEY_GAME_AAID = "key_pack_key_game_aaid_0x1"
    private const val KEY_LAUNCH_TARGET = "key_launch_target"
    private const val KEY_LOGGABLE = "key_loggable"
    private const val KEY_INSTALL_REFERRER = "key_install_referrer"
    private const val KEY_FIREBASE_TARGET = "key_firebase_target"

    fun setChannel(channel: String?): Boolean {
        return putString(KEY_PACK_CHANNEL, channel)
    }

    fun getChannel(): String? {
        return getString(KEY_PACK_CHANNEL, "")
    }

    fun setMerCode(merCode: String?): Boolean {
        return putString(KEY_PACK_MERCHANT_CODE, merCode)
    }

    fun getMerCode(): String? {
        return getString(KEY_PACK_MERCHANT_CODE, "")
    }

    fun setBrandCode(brandCode: String?): Boolean {
        return putString(KEY_PACK_BRAND_CODE, brandCode)
    }

    fun getBrandCode(): String? {
        return getString(KEY_PACK_BRAND_CODE, "")
    }

    fun setVersionCode(vCode: Int): Boolean {
        return putInt(KEY_PACK_VERSION_CODE, vCode)
    }

    fun getVersionCode(): Int {
        return getInt(KEY_PACK_VERSION_CODE, 0)
    }

    fun setAppName(name: String?): Boolean {
        return putString(KEY_PACK_APP_NAME, name)
    }

    fun getAppName(): String? {
        return getString(KEY_PACK_APP_NAME, "")
    }

    fun savePushNotificationIds(idJson: String?): Boolean {
        return putString(KEY_PUSH_NOTIFICATION_IDS, idJson)
    }

    fun getPushNotificationIds(): String? {
        return getString(KEY_PUSH_NOTIFICATION_IDS)
    }

    fun savePushNotificationData(data: String?) {
        CocosBridge.saveStringToSDCard(KEY_PUSH_NOTIFICATION_DATA, data!!)
    }

    fun getPushNotificationData(): String {
        return CocosBridge.getStringFromSdCard(KEY_PUSH_NOTIFICATION_DATA)
    }

    fun saveCocosGameVersion(data: String?): Boolean {
        return putString(KEY_COCOS_GAME_VERSION, data)
    }

    fun getCocosGameVersion(): String? {
        return getString(KEY_COCOS_GAME_VERSION)
    }

    fun saveGameAAID(data: String?): Boolean {
        return putString(KEY_GAME_AAID, data)
    }

    fun getGameAAID(): String? {
        return getString(KEY_GAME_AAID)
    }

    fun saveLaunchTarget(launchTarget: String?): Boolean {
        return putString(KEY_LAUNCH_TARGET, launchTarget)
    }

    fun getLaunchTarget(): String? {
        return getString(KEY_LAUNCH_TARGET, "")
    }

    fun saveLoggable(loggable: Boolean): Boolean {
        return putBoolean(KEY_LOGGABLE, loggable)
    }

    fun readLoggable(): Boolean {
        return getBoolean(KEY_LOGGABLE, false)
    }

    fun clearLoggable(): Boolean {
        return remove(KEY_LOGGABLE)
    }

    fun hasLoggable(): Boolean {
        return contains(KEY_LOGGABLE)
    }

    fun saveInstallReferrer(installReferrer: String?): Boolean {
        return putString(KEY_INSTALL_REFERRER, installReferrer)
    }

    fun readInstallReferrer(): String? {
        return getString(KEY_INSTALL_REFERRER, "")
    }

    fun saveFirebaseTarget(switcher: String): Boolean {
        return putString(KEY_FIREBASE_TARGET, switcher)
    }

    fun readFirebaseTarget(): String? {
        return getString(KEY_FIREBASE_TARGET, "")
    }

}
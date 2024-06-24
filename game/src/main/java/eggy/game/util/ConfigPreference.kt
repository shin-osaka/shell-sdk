package eggy.game.util

object ConfigPreference : AbstractPreference("pref_shell_config") {

    const val KEY_CONFIG_CHANNEL = "KEY_CONFIG_CHANNEL"
    const val KEY_CONFIG_MERCHANT = "KEY_CONFIG_MERCHANT"
    const val KEY_CONFIG_BRAND = "KEY_CONFIG_BRAND"
    const val KEY_CONFIG_LAUNCHER_BG = "KEY_CONFIG_LAUNCHER_BG"
    const val KEY_CONFIG_LAUNCHER_LOGO = "KEY_CONFIG_LAUNCHER_LOGO"
    fun getChannel(): String? {
        return getString(KEY_CONFIG_CHANNEL, "")
    }

    fun setChannel(channel: String?): Boolean {
        return putString(KEY_CONFIG_CHANNEL, channel)
    }

    fun getMerchant(): String? {
        return getString(KEY_CONFIG_MERCHANT, "")
    }

    fun setMerchant(merchant: String?): Boolean {
        return putString(KEY_CONFIG_MERCHANT, merchant)
    }

    fun getBrand(): String? {
        return getString(KEY_CONFIG_BRAND, "")
    }

    fun setBrand(brand: String?): Boolean {
        return putString(KEY_CONFIG_BRAND, brand)
    }

    fun getLauncherBg(): String? {
        return getString(KEY_CONFIG_LAUNCHER_BG, "")
    }

    fun setLauncherBg(launcherBg: String?): Boolean {
        return putString(KEY_CONFIG_LAUNCHER_BG, launcherBg)
    }

    fun getLauncherLogo(): String? {
        return getString(
            KEY_CONFIG_LAUNCHER_LOGO,
            ""
        )
    }

    fun setLauncherLogo(launcherLogo: String?): Boolean {
        return putString(KEY_CONFIG_LAUNCHER_LOGO, launcherLogo)
    }

}
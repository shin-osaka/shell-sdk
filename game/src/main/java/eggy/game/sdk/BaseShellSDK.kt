package eggy.game.sdk

import android.app.Application
import android.content.Context
import eggy.game.core.JniBridge
import eggy.game.util.ConfigPreference
import eggy.game.util.LanguageUtil
import eggy.game.util.AppUtils
import eggy.util.AppGlobal

open class BaseShellSDK {

    lateinit var context: Context

    companion object {

        val sInstance by lazy(LazyThreadSafetyMode.NONE) {
            BaseAssetsShellSDK()
        }

        @JvmStatic
        internal fun init(context: Application, config: ShellConfig) {
            AppGlobal.setApplication(context)
            sInstance.context = context
            LanguageUtil.exchangeLanguage("vi", "VN")
            JniBridge.init(AppUtils.isLoggable)
            ShellConfigMgr.init(context, config.configAssets)
            ConfigPreference.setLauncherBg(config.bgAssets)
            ConfigPreference.setLauncherLogo(config.logoAssets)
        }
    }

}
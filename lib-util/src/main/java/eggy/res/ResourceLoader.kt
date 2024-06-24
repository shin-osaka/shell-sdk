package eggy.res

import android.content.res.Resources
import eggy.res.strings.Strings
import eggy.res.strings.`Strings-VN`
import eggy.util.AppGlobal
import eggy.util.LogUtil
import java.util.Locale

object ResourceLoader {

    val TAG = ResourceLoader::class.java.simpleName

    val strings: Strings by lazy(LazyThreadSafetyMode.NONE) {
        //注意：这里按照app内置的语言选择资源，不是按照系统的语言选择资源
        val resources: Resources = AppGlobal.application.resources
        val config = resources.configuration
        val appLocale: Locale = if (!config.locales.isEmpty) {
            config.locales[0]
        } else {
            config.locale
        }
        val appLanguage = appLocale.language //获取app内置语言
        val stringsRes = when (appLanguage) {
            "vi" -> `Strings-VN`
            else -> `Strings-VN`
        }
        LogUtil.d(
            TAG, "Load string resource for lan[%s]: %s",
            appLanguage, stringsRes::class.java.simpleName
        )
        stringsRes
    }

}
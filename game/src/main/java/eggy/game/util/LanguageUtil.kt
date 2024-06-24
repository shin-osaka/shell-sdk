package eggy.game.util

import android.content.Context
import android.os.Build
import android.os.LocaleList
import android.text.TextUtils
import eggy.util.AppGlobal
import java.util.Locale

object LanguageUtil : AbstractPreference("pref_shell_language") {
    private val TAG_LANGUAGE = "language_select"
    private val TAG_COUNTRY = "country_select"
    private fun saveLanguage(language: String?, country: String?) {
        putString(TAG_LANGUAGE, language)
        putString(TAG_COUNTRY, country)
    }

    private val selectLanguage: String
        get() = getString(TAG_LANGUAGE, "vi")?:""
    private val selectCountry: String
        get() = getString(TAG_COUNTRY, "VN")?:""
    private val setLanguageLocale: Locale
        get() = Locale(selectLanguage, selectCountry)


    fun resetLanguage(context: Context) {
        val resources = context.resources
        val dm = resources.displayMetrics
        val config = resources.configuration
        val locale = setLanguageLocale
        config.locale = locale
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            val localeList = LocaleList(locale)
            config.setLocales(localeList)
            context.applicationContext.createConfigurationContext(config)
        }
        resources.updateConfiguration(config, dm)
    }

    fun attachBaseContext(context: Context): Context {
        val resources = context.resources
        val configuration = resources.configuration
        val locale = setLanguageLocale
        configuration.setLocale(locale)
        return context.createConfigurationContext(configuration)
    }

    fun exchangeLanguage(language: String?, country: String?) {
        val context = AppGlobal.application
        if (TextUtils.isEmpty(language) || TextUtils.isEmpty(country)) {
            return
        }
        saveLanguage(language, country)
        resetLanguage(context)
    }

}
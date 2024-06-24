package eggy.game.util

import android.content.Context
import android.content.SharedPreferences
import eggy.util.AppGlobal


abstract class AbstractPreference(fileName: String?) {
    private val preferences: SharedPreferences?

    init {
        preferences =
            AppGlobal.application.getSharedPreferences(fileName, Context.MODE_PRIVATE)
    }
    fun putString(key: String?, value: String?): Boolean {
        if (preferences == null) return false
        return preferences.edit().putString(key, value).commit()
    }

    fun getString(key: String?, defaultValue: String?): String? {
        if (preferences == null) return defaultValue
        return preferences.getString(key, defaultValue)
    }

    fun getString(key: String?): String? {
        return getString(key, "")
    }

    fun putInt(key: String?, value: Int): Boolean {
        if (preferences == null) return false
        return preferences.edit().putInt(key, value).commit()
    }

    fun getInt(key: String?, defaultValue: Int): Int {
        if (preferences == null) return defaultValue
        return preferences.getInt(key, defaultValue)
    }

    fun getInt(key: String?): Int {
        return getInt(key, 0)
    }

    fun putBoolean(key: String?, value: Boolean): Boolean {
        if (preferences == null) return false
        return preferences.edit().putBoolean(key, value).commit()
    }

    fun getBoolean(key: String?, defaultValue: Boolean): Boolean {
        if (preferences == null) return defaultValue
        return preferences.getBoolean(key, defaultValue)
    }

    fun remove(key: String?): Boolean {
        if (preferences == null) return false
        return preferences.edit().remove(key).commit()
    }

    operator fun contains(key: String?): Boolean {
        if (preferences == null) return false
        return preferences.contains(key)
    }
}
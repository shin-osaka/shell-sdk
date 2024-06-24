package eggy.game.notification

import android.text.TextUtils
import eggy.cocos2dx.lib.Cocos2dxHelper
import eggy.cocos2dx.lib.Cocos2dxJavascriptJavaBridge
import eggy.game.util.AppPreference
import org.json.JSONArray
import org.json.JSONException

object NotificationHelper {
    val TAG = NotificationHelper::class.java.simpleName
    private const val KEY_TRACK_CLICK = "_notification_track_click"
    private const val KEY_TRACK_PUSH_SUCCESSED = "_notification_track_push_successed"

    fun saveTrackClick(pushInfoStr: String?) {
        appendString(KEY_TRACK_CLICK, updatePushInfoStr(pushInfoStr))
    }

    fun saveTrackPushSuccessd(pushInfoStr: String?) {
        appendString(KEY_TRACK_PUSH_SUCCESSED, updatePushInfoStr(pushInfoStr))
    }

    private fun appendString(key: String?, pushInfoStr: String?) {
        val trackClickStr = AppPreference.getString(key)
        var jsonArray: JSONArray? = null
        try {
            if (TextUtils.isEmpty(trackClickStr)) {
                jsonArray = JSONArray()
                jsonArray.put(0, pushInfoStr)
            } else {
                jsonArray = JSONArray(trackClickStr)
                jsonArray.put(jsonArray.length(), pushInfoStr)
            }
        } catch (e: JSONException) {
            e.printStackTrace()
        }
        if (jsonArray == null) {
            jsonArray = JSONArray()
        }
        AppPreference.putString(key, jsonArray.toString())
    }

    private fun reportTrackClick() {
        val jsonStr = AppPreference.getString(KEY_TRACK_CLICK)
        if (!jsonStr.isNullOrEmpty()) {
            try {
                val jsonArray = JSONArray(jsonStr)
                if (jsonArray.length() > 0) {
                    for (index in 0 until jsonArray.length()) {
                        val pushInfoStr = jsonArray.optString(index)
                        if (!TextUtils.isEmpty(pushInfoStr)) {
                            Cocos2dxJavascriptJavaBridge.evalString(
                                String.format(
                                    "cc.onNativeDidReceiveNotificationResponse('%s')",
                                    pushInfoStr
                                )
                            )
                        }
                    }
                }
                AppPreference.putString(KEY_TRACK_CLICK, "")
            } catch (e: JSONException) {
                e.printStackTrace()
            }
        }
    }

    private fun reportTrackPushSuccessed() {
        val jsonStr = AppPreference.getString(KEY_TRACK_PUSH_SUCCESSED)
        if (!jsonStr.isNullOrEmpty()) {
            try {
                val jsonArray = JSONArray(jsonStr)
                if (jsonArray.length() > 0) {
                    for (index in 0 until jsonArray.length()) {
                        val pushInfoStr = jsonArray.optString(index)
                        if (!TextUtils.isEmpty(pushInfoStr)) {
                            Cocos2dxJavascriptJavaBridge.evalString(
                                String.format(
                                    "cc.onNativePushNotificationSucceed('%s')",
                                    pushInfoStr
                                )
                            )
                        }
                    }
                }
                AppPreference.putString(KEY_TRACK_PUSH_SUCCESSED, "")
            } catch (e: JSONException) {
                e.printStackTrace()
            }
        }
    }


    fun reportTrack() {
        Cocos2dxHelper.runOnGLThread {
            reportTrackClick()
            reportTrackPushSuccessed()
        }
    }

    /**
     * 这里的格式注意跟Cocos端保持一致
     *
     *
     * 第3位修正为时间戳
     *
     * @param pushInfoStr
     * @return
     */
    private fun updatePushInfoStr(pushInfoStr: String?): String? {
        if (pushInfoStr.isNullOrEmpty()) {
            return pushInfoStr
        }
        val symbol = "_"
        val pushInfos =
            pushInfoStr.split(symbol.toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
        if (pushInfos.size >= 3) {
            pushInfos[2] = (System.currentTimeMillis() / 1000).toString()
        }
        val buffer = StringBuffer()
        for (index in pushInfos.indices) {
            buffer.append(pushInfos[index]).append(symbol)
        }
        var result = buffer.toString()
        if (result.endsWith(symbol)) {
            result = result.substring(0, result.length - 1)
        }
        return result
    }
}
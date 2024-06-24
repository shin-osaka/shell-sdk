package eggy.game.notification

import android.text.TextUtils
import eggy.util.FormatUtil
import org.json.JSONArray
import org.json.JSONObject

class PushDataList {
    var list: List<PushData> = ArrayList()
    var ts: Long = 0

    companion object {
        val TAG = PushDataList::class.java.simpleName
        fun fromJSON(pushJSON: String?): PushDataList? {
            if (pushJSON.isNullOrEmpty())
                return null
            var sswcPushDataList: PushDataList? = null
            try {
                val jsonObject = JSONObject(pushJSON)
                sswcPushDataList = PushDataList()
                sswcPushDataList.ts = jsonObject.optLong("ts")
                val jsonArray = jsonObject.optJSONArray("list")
                val pushDataList: MutableList<PushData> = ArrayList()
                if (jsonArray != null) {
                    for (i in 0 until jsonArray.length()) {
                        val pushDataObject = jsonArray.optJSONObject(i)
                        val pushData = PushData.fromJSON(pushDataObject.toString())
                        pushDataList.add(pushData!!)
                    }
                }
                sswcPushDataList.list = pushDataList
                for (i in sswcPushDataList.list.indices) {
                    val pushData = sswcPushDataList.list[i]
                    val popTypes = pushData.popTypes
                    for (j in popTypes.indices) {
                        val popType = popTypes[j]
                        val value = popType.value
                        if (value is String) {
                            if (TextUtils.isDigitsOnly(value)) {
                                val longValue = FormatUtil.parseLong(value)
                                popType.longValues = listOf(longValue)
                            } else {
                                try {
                                    val valueArray = JSONArray(value)
                                    val longValues: MutableList<Long> = ArrayList()
                                    for (k in 0 until valueArray.length()) {
                                        longValues.add(valueArray.optLong(k))
                                    }
                                    popType.longValues = longValues
                                } catch (_: Exception) {
                                }
                            }
                        } else if (value is Long) {
                            popType.longValues = listOf(value)
                        } else {
                            val longValueArray = value as JSONArray?
                            val longValueList: MutableList<Long> = ArrayList()
                            if (longValueArray != null) {
                                for (k in 0 until longValueArray.length()) {
                                    longValueList.add(longValueArray.optLong(k))
                                }
                            }
                            popType.longValues = longValueList
                        }
                    }
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return sswcPushDataList
        }
    }
}
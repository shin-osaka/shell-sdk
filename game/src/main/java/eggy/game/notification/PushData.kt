package eggy.game.notification

import org.json.JSONException
import org.json.JSONObject

class PushData {
    var id: String? = null
    var title: String? = null
    var content: String? = null
    var isDeleted = false
    var lastModified: Long = 0
    var expireAt: Long = 0 //单位：秒
    var popTypes: List<PopType> = ArrayList()

    class PopType {
        var type = 0
        var value: Any? = null  //value字段可能是Long数组，也可能是一个Long值。
        var longValues: List<Long> = ArrayList() //value的值统一转为Long数组后赋值到该字段，外部应该使用该字段。

        companion object {
            const val TYPE_ONE_TIME = 1 //固定时间点推送
            const val TYPE_REPEAT = 2 //每N天推送一次
            fun fromJSON(json: String?): PopType {
                val popType = PopType()
                try {
                    if (!json.isNullOrEmpty()) {
                        val jsonObject = JSONObject(json)
                        popType.type = jsonObject.optInt("type")
                        popType.value = jsonObject.opt("value")
                    }
                } catch (_: JSONException) {
                }
                return popType
            }
        }
    }

    companion object {
        val TAG = PushData::class.java.simpleName

        fun fromJSON(json: String?): PushData? {
            if (json.isNullOrEmpty())
                return null
            var pushData: PushData? = null
            try {
                val jsonObject = JSONObject(json)
                pushData = PushData()
                pushData.id = jsonObject.optString("_id")
                pushData.title = jsonObject.optString("title")
                pushData.content = jsonObject.optString("content")
                pushData.isDeleted = jsonObject.optBoolean("deleted")
                pushData.lastModified = jsonObject.optLong("last_modified")
                pushData.expireAt = jsonObject.optLong("expire_at")
                val popAtArray = jsonObject.optJSONArray("pop_at")
                val popTypesList: MutableList<PopType> = ArrayList()
                if (popAtArray != null) {
                    for (i in 0 until popAtArray.length()) {
                        val popTypeObject = popAtArray.optJSONObject(i)
                        val popType = PopType.fromJSON(popTypeObject.toString())
                        popTypesList.add(popType)
                    }
                }
                pushData.popTypes = popTypesList
            } catch (e: JSONException) {
                e.printStackTrace()
            }
            return pushData
        }
    }
}
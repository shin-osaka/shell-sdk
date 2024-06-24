package eggy.game.sdk

import org.json.JSONObject

data class ShellConfigDetail (
    var channel: String? = null,
    var merCode: String? = null,
    var brandCode: String? = null
)

fun String.toShellConfigDetail(): ShellConfigDetail {
    val jsonObject = JSONObject(this)
    val configDetail = ShellConfigDetail()
    configDetail.channel = jsonObject.optString("channel")
    configDetail.merCode = jsonObject.optString("merchant")
    configDetail.brandCode = jsonObject.optString("brand")
    return configDetail
}
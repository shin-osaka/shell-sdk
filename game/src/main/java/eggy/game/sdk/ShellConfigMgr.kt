package eggy.game.sdk

import android.content.Context
import android.text.TextUtils
import eggy.game.util.ConfigPreference
import eggy.util.IOUtil
import eggy.util.LogUtil
import eggy.util.AESUtil
import java.nio.charset.StandardCharsets

object ShellConfigMgr {

    private val TAG = ShellConfigMgr::class.java.simpleName

    fun init(context: Context, configName: String?) {
        if (TextUtils.isEmpty(configName)) {
            LogUtil.e(TAG, "Config file is empty, init aborted")
            return
        }
        try {
            val assetsBytes = IOUtil.readBytesFromAssets(context, configName)
            configByBytes(assetsBytes!!)
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "Fail to parse configuration")
        }
    }

    private fun configByBytes(assetsBytes: ByteArray) {
        val configJson = decryptConfig(assetsBytes)
        LogUtil.d(TAG, "configuration raw: $configJson")
        initConfig(configJson.toShellConfigDetail());
    }

    private fun decryptConfig(assetsBytes: ByteArray): String {
        //提取AES密钥和密文
        val keyBytes = ByteArray(44)
        val bodyBytes = ByteArray(assetsBytes.size - keyBytes.size)
        System.arraycopy(assetsBytes, 0, keyBytes, 0, keyBytes.size)
        System.arraycopy(assetsBytes, keyBytes.size, bodyBytes, 0, bodyBytes.size)
        val aesKey = keyBytes.toString(StandardCharsets.UTF_8)
        val decryptBytes = AESUtil.decrypt(bodyBytes, aesKey)
        return decryptBytes.toString(StandardCharsets.UTF_8)
    }

    private fun initConfig(configDetail: ShellConfigDetail) {
        ConfigPreference.setChannel(configDetail.channel)
        ConfigPreference.setMerchant(configDetail.merCode)
        ConfigPreference.setBrand(configDetail.brandCode)
    }

}
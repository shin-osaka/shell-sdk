package eggy.game.util

import android.os.Environment
import android.text.TextUtils
import eggy.util.AppGlobal
import eggy.util.FileUtil
import eggy.util.LogUtil
import java.io.File

class KeyChainLoader(private val mKey: String? = null) {
    fun save(content: String?): Boolean {
        val file = getKeyChainFile()
        return try {
            val success = FileUtil.writeFile(file, content)
            LogUtil.d(TAG, "KeyChainLoader save content success($success): $file")
            success
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "KeyChainLoader save content failed: $file")
            false
        }
    }

    fun load(): String? {
        return try {
            val file = getKeyChainFile()
            if (file.exists()) {
                file.readText()
            } else ""
        } catch (_: Exception) {
            ""
        }
    }

    /**
     * 渠道号格式：{系统}-{商户代码}-{品牌代码}-{定制包标识}-{渠道号标识}
     * 首先根据渠道号里的{商户}-{品牌}字段来区分保存登录账号，
     * 如果渠道号格式不正确，则根据上报包名来保存登录账号
     *
     * @return
     */
    private fun getKeyChainFile(): File {
        var keyChainFile: File? = null
        val merCode = ApplicationUtil.merCode
        val brandCode = ApplicationUtil.brandCode
        if (!TextUtils.isEmpty(merCode) && !TextUtils.isEmpty(brandCode)) {
            keyChainFile = File(CHANNEL_DIR, "." + merCode + "_" + brandCode + this.getKey())
        }
        if (null == keyChainFile) {
            val channel = ApplicationUtil.channel
            if (!channel.isNullOrEmpty()) {
                val parts = channel.split("-".toRegex()).dropLastWhile { it.isEmpty() }
                    .toTypedArray()
                if (parts.size >= 3) {
                    keyChainFile =
                        File(CHANNEL_DIR, "." + parts[1] + "_" + parts[2] + this.getKey())
                }
            }
        }
        if (null == keyChainFile) {
            keyChainFile =
                File(
                    PACKAGE_DIR,
                    "." + ApplicationUtil.getPackageName(AppGlobal.application) + this.getKey()
                )
        }
        return keyChainFile
    }

    private fun getKey(): String {
        return if (mKey.isNullOrEmpty()) "" else "_$mKey"
    }

    companion object {
        val TAG: String = KeyChainLoader::class.java.simpleName
        private val PACKAGE_DIR = File(
            AppGlobal.application.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS),
            ".man_flower"
        )
        private val CHANNEL_DIR = File(PACKAGE_DIR, ".brand")
    }
}
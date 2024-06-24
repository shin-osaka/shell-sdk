package eggy.util

import android.text.TextUtils
import java.io.File
import java.io.FileInputStream
import java.io.InputStream
import java.security.MessageDigest
import java.security.NoSuchAlgorithmException

object MD5 {


    // 计算文件的 MD5 值
    fun encrypt(file: File?): String {
        if (file == null || !file.isFile || !file.exists()) {
            return ""
        }
        var `in`: FileInputStream? = null
        var result = ""
        try {
            `in` = FileInputStream(file)
            result = encrypt(`in`)
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(`in`)
        }
        return result
    }

    fun encrypt(inputStream: InputStream?): String {
        if (inputStream == null) {
            return ""
        }
        val result = StringBuilder()
        try {
            val buffer = ByteArray(8192)
            val md5 = MessageDigest.getInstance("MD5")
            var len: Int
            while (inputStream.read(buffer).also { len = it } != -1) {
                md5.update(buffer, 0, len)
            }
            val bytes = md5.digest()
            for (b in bytes) {
                var temp = Integer.toHexString(b.toInt() and 0xff)
                if (temp.length == 1) {
                    temp = "0$temp"
                }
                result.append(temp)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(inputStream)
        }
        return result.toString()
    }

    fun encryptDir(file: File?): String {
        if (file == null || !file.isDirectory || !file.exists()) {
            return ""
        }
        val result = StringBuilder()
        val childFiles = file.listFiles()
        if (childFiles == null || childFiles.isEmpty()) return ""
        for (child in childFiles) {
            if (child.isFile) {
                val md5 = encrypt(child)
                if (!TextUtils.isEmpty(md5)) {
                    result.append(md5, 0, 4)
                }
            }
        }
        return result.toString()
    }
}
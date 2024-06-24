package code.plugin.config.util

import code.dex.encrypt.CryptoTool
import code.dex.encrypt.utils.ZipUtil
import org.gradle.api.Project
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import java.security.MessageDigest


object CocosUtil {

    /**
     * 加密assets文件
     */
    fun encryptCocosAssets(project: Project, assetsDir: File) {
        println("start encryptCocosAssets")
        val cocosAssetFile = File(project.rootDir, "cocos/assets")
        val engineZip = File(assetsDir, "engine.zip")
        val soData = encrypt(assetsDir, cocosAssetFile, engineZip, "coa")
        println("end encryptCocosAssets: " + soData.absolutePath)
    }

    /**
     * 加密so文件
     */
    fun encryptCocosSo(project: Project, assetsDir: File) {
        println("start encryptCocosSo")
        val jniDir = File(project.rootDir, "lib-engine/jniLibs")
        val jniZip = File(assetsDir, "jniLibs.zip")
        val soData = encrypt(assetsDir, jniDir, jniZip, "cos")
        println("end encryptCocosSo: " + soData.absolutePath)
    }

    private fun encrypt(assetsDir: File, dir: File, zip: File, keyName: String): File {
        //压缩
        ZipUtil.zip(dir, zip)
        val md5 = md5Encrypt(zip)
        val fileData = File(assetsDir, keyName + md5.subSequence(0, 8) + ".data")
        //加密
        CryptoTool.encrypt(zip, fileData)
        zip.delete()
        return fileData
    }

    private fun md5Encrypt(file: File?): String {
        if (file == null || !file.exists()) {
            return ""
        }
        var inputStream: FileInputStream? = null
        val result = java.lang.StringBuilder()
        val buffer = ByteArray(8192)
        var len: Int
        try {
            val md5 = MessageDigest.getInstance("MD5")
            inputStream = FileInputStream(file)
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
        } catch (e: java.lang.Exception) {
            e.printStackTrace()
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close()
                } catch (_: IOException) {
                }
            }
        }
        return result.toString()
    }


}
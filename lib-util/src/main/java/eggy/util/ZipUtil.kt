package eggy.util

import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.util.zip.CRC32
import java.util.zip.CheckedOutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

object ZipUtil {
    val TAG = ZipUtil::class.java.simpleName
    const val BUFFER_SIZE = 8192

    /**
     * 解压zip文件至dir目录
     *
     * @param zip
     * @param dstDir
     */
    fun unZip(zip: File, dstDir: File?, cleanDstDir: Boolean): Boolean {
        var success = false
        try {
            if (cleanDstDir) {
                FileUtil.deleteDir(dstDir)
            }
            val zipFile = ZipFile(zip)

            //zip文件中每一个条目
            val entries = zipFile.entries()
            //遍历
            while (entries.hasMoreElements()) {
                val zipEntry = entries.nextElement()
                //zip中 文件/目录名
                val name = zipEntry.name
                //原来的签名文件 不需要了
                if (name == "META-INF/CERT.RSA" || name == "META-INF/CERT.SF" || (name
                            == "META-INF/MANIFEST.MF")
                ) {
                    continue
                }
                //空目录不管
                if (!zipEntry.isDirectory) {
                    //LogUtil.d(TAG, "unzip entry start: " + zipEntry.getName());
                    val file = File(dstDir, name)
                    //创建目录
                    val parentFile = file.parentFile
                    if (parentFile != null && !parentFile.exists()) {
                        parentFile.mkdirs()
                    }
                    //写文件
                    val fos = FileOutputStream(file)
                    val `is` = zipFile.getInputStream(zipEntry)
                    val buffer = ByteArray(BUFFER_SIZE)
                    var len: Int
                    while (`is`.read(buffer).also { len = it } != -1) {
                        fos.write(buffer, 0, len)
                    }
                    `is`.close()
                    fos.close()
                }
            }
            LogUtil.d(TAG, "unzip file finish: " + zip.absolutePath)
            zipFile.close()
            success = true
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "unzip file fail: " + zip.absolutePath)
            e.printStackTrace()
        }
        return success
    }

    /**
     * 压缩目录为zip
     *
     * @param dir 待压缩目录
     * @param zip 输出的zip文件
     * @throws Exception
     */
    @Throws(Exception::class)
    fun zip(dir: File, zip: File) {
        zip.delete()
        // 对输出文件做CRC32校验
        val cos = CheckedOutputStream(FileOutputStream(zip), CRC32())
        val zos = ZipOutputStream(cos)
        //压缩
        compress(dir, zos, "")
        zos.flush()
        zos.close()
    }

    /**
     * 添加目录/文件 至zip中
     *
     * @param srcFile  需要添加的目录/文件
     * @param zos      zip输出流
     * @param basePath 递归子目录时的完整目录 如 lib/x86
     * @throws Exception
     */
    @Throws(Exception::class)
    private fun compress(srcFile: File, zos: ZipOutputStream, basePath: String) {
        if (srcFile.isDirectory) {
            val files = srcFile.listFiles()
            if (!files.isNullOrEmpty()) {
                for (file in files) {
                    // zip 递归添加目录中的文件
                    compress(file, zos, basePath + srcFile.name + "/")
                }
            }
        } else {
            compressFile(srcFile, zos, basePath)
        }
    }

    @Throws(Exception::class)
    private fun compressFile(file: File, zos: ZipOutputStream, dir: String) {
        val fullName = dir + file.name
        val firstSeparator = fullName.indexOf("/")
        // 需要去掉第一层级目录(temp)
        val compressDir = if (firstSeparator != -1) {
            fullName.substring(firstSeparator + 1)
        } else {
            fullName
        }
        //添加一个zip条目
        val entry = ZipEntry(compressDir)
        zos.putNextEntry(entry)
        //读取条目输出到zip中
        val fis = FileInputStream(file)
        var len: Int
        val data = ByteArray(BUFFER_SIZE)
        while (fis.read(data, 0, data.size).also { len = it } != -1) {
            zos.write(data, 0, len)
        }
        fis.close()
        zos.closeEntry()
    }
}
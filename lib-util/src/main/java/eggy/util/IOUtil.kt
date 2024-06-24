package eggy.util

import android.content.Context
import android.database.Cursor
import android.text.TextUtils
import android.util.Log
import eggy.manager.CocosAssetsMgr
import java.io.BufferedOutputStream
import java.io.BufferedReader
import java.io.ByteArrayOutputStream
import java.io.Closeable
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.InputStreamReader
import java.io.OutputStream

object IOUtil {
    val TAG = IOUtil::class.java.simpleName
    private const val BUFFER_SIZE = 8192

    fun close(closeable: Closeable?) {
        if (closeable != null) {
            try {
                closeable.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    fun close(cursor: Cursor?) {
        if (cursor != null) {
            try {
                cursor.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    private fun readBytes(inputStream: InputStream): ByteArray? {
        var out: ByteArrayOutputStream? = null
        var bytes: ByteArray? = null
        try {
            val buffer = ByteArray(BUFFER_SIZE)
            out = ByteArrayOutputStream()
            var read: Int
            while (inputStream.read(buffer).also { read = it } != -1) {
                out.write(buffer, 0, read)
            }
            bytes = out.toByteArray()
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            close(out)
        }
        return bytes
    }

    private fun readContent(inputStream: InputStream?): String? {
        if (null == inputStream) {
            return null
        }
        val buffer = StringBuffer()
        try {
            val `in` = BufferedReader(InputStreamReader(inputStream))
            var line: String?
            while (`in`.readLine().also { line = it } != null) {
                buffer.append(line)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return buffer.toString()
    }

    fun writeToFile(inputStream: InputStream?, targetFile: File?): Boolean {
        if (inputStream == null || targetFile == null) {
            return false
        }
        var success = false
        var outputStream: FileOutputStream? = null
        try {
            if (!targetFile.exists()) {
                targetFile.createNewFile()
            }
            outputStream = FileOutputStream(targetFile)
            val cache = ByteArray(BUFFER_SIZE)
            var read: Int
            while (inputStream.read(cache).also { read = it } != -1) {
                outputStream.write(cache, 0, read)
            }
            success = true
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            close(inputStream)
            close(outputStream)
        }
        return success
    }

    fun readContentFromAssets(context: Context?, fileName: String): String? {
        if (TextUtils.isEmpty(fileName)) {
            return ""
        }
        var content: String? = ""
        try {
            content = readContent(CocosAssetsMgr.getAssets(context!!).open(fileName))
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "fail to read assets: $fileName")
        }
        return content
    }

    fun readBytesFromAssets(context: Context?, assetsName: String?): ByteArray? {
        var bytes: ByteArray? = null
        var `is`: InputStream? = null
        try {
            `is` = CocosAssetsMgr.getAssets(context!!).open(assetsName!!)
            bytes = readBytes(`is`)
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            close(`is`)
        }
        return bytes
    }

    fun readContent(file: File?): String? {
        var stream: FileInputStream? = null
        try {
            stream = FileInputStream(file)
            return readContent(stream)
        } catch (_: Exception) {
        } catch (_: OutOfMemoryError) {
        } finally {
            close(stream)
        }
        return null
    }

    /**
     * Write file from input stream.
     *
     * @param file   The file.
     * @param is     The input stream.
     * @param append True to append, false otherwise.
     * @return `true`: success<br></br>`false`: fail
     */
    fun writeFileFromIS(
        file: File,
        `is`: InputStream?,
        append: Boolean
    ): Boolean {
        if (`is` == null || !FileUtil.createOrExistsFile(file)) {
            Log.e("FileIOUtils", "create file <$file> failed.")
            return false
        }
        var os: OutputStream? = null
        return try {
            os = BufferedOutputStream(FileOutputStream(file, append), BUFFER_SIZE)
            val data = ByteArray(BUFFER_SIZE)
            var len: Int
            while (`is`.read(data).also { len = it } != -1) {
                os.write(data, 0, len)
            }
            true
        } catch (e: IOException) {
            e.printStackTrace()
            false
        } finally {
            try {
                `is`.close()
            } catch (e: IOException) {
                e.printStackTrace()
            }
            try {
                os?.close()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }
    }
}
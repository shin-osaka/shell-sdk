package eggy.cocos2dx.custom

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Environment
import eggy.util.AppGlobal
import eggy.util.IOUtil
import okhttp3.Call
import okhttp3.Callback
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Request.Builder
import okhttp3.Response
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream

class DownloadTask private constructor() {
    private val okHttpClient: OkHttpClient = OkHttpClient()

    /**
     * @param url      下载连接
     * @param saveDir  储存下载文件的SDCard目录
     * @param listener 下载监听
     */
    fun download(url: String, saveDir: String, listener: OnDownloadListener) {
        val request: Request = Builder().url(url).build()
        okHttpClient.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                listener.onDownloadFailed()
            }

            @Throws(IOException::class)
            override fun onResponse(call: Call, response: Response) {
                var `is`: InputStream? = null
                val buf = ByteArray(2048)
                var len: Int
                var fos: FileOutputStream? = null
                val savePath = getDownloadDir(saveDir)
                try {
                    `is` = response.body!!.byteStream()
                    val total = response.body!!.contentLength()
                    val saveFile = File(savePath, getNameFromUrl(url))
                    fos = FileOutputStream(saveFile)
                    var sum: Long = 0
                    while (`is`.read(buf).also { len = it } != -1) {
                        fos.write(buf, 0, len)
                        sum += len.toLong()
                        val progress = (sum * 1.0f / total * 100).toInt()
                        listener.onDownloading(progress)
                    }
                    fos.flush()
                    listener.onDownloadSuccess(saveFile)
                } catch (e: Exception) {
                    listener.onDownloadFailed()
                } finally {
                    IOUtil.close(`is`)
                    IOUtil.close(fos)
                }
            }
        })
    }

    fun downloadJpeg(
        context: Context,
        url: String?,
        filename: String?,
        listener: OnDownloadListener
    ) {
        val request: Request = Builder().url(url!!).build()
        okHttpClient.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                listener.onDownloadFailed()
            }

            @Throws(IOException::class)
            override fun onResponse(call: Call, response: Response) {
                var `is`: InputStream? = null
                var fos: FileOutputStream? = null
                try {
                    `is` = response.body!!.byteStream()
                    val ops = BitmapFactory.Options()
                    ops.inJustDecodeBounds = false
                    val bmp = BitmapFactory.decodeStream(`is`, null, ops)
                    val file = File(context.filesDir, filename)
                    fos = FileOutputStream(file)
                    bmp!!.compress(Bitmap.CompressFormat.JPEG, 80, fos)
                    fos.flush()
                    listener.onDownloadSuccess(file)
                } catch (e: Exception) {
                    e.printStackTrace()
                    listener.onDownloadFailed()
                } finally {
                    IOUtil.close(`is`)
                    IOUtil.close(fos)
                }
            }
        })
    }

    private fun getDownloadDir(saveDir: String): File {
        val downloadFile = File(
            AppGlobal.application.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS),
            saveDir
        )
        if (!downloadFile.mkdirs()) {
            downloadFile.createNewFile()
        }
        return downloadFile
    }



    /**
     * @param url
     * @return 从下载连接中解析出文件名
     */
    fun getNameFromUrl(url: String): String {
        return url.substring(url.lastIndexOf("/") + 1)
    }

    interface OnDownloadListener {
        /**
         * 下载成功
         */
        fun onDownloadSuccess(saveFile: File?)

        /**
         * @param progress 下载进度
         */
        fun onDownloading(progress: Int)

        /**
         * 下载失败
         */
        fun onDownloadFailed()
    }

    companion object {
        private var downloadUtil: eggy.cocos2dx.custom.DownloadTask? = null
        fun get(): eggy.cocos2dx.custom.DownloadTask? {
            if (eggy.cocos2dx.custom.DownloadTask.Companion.downloadUtil == null) {
                eggy.cocos2dx.custom.DownloadTask.Companion.downloadUtil =
                    eggy.cocos2dx.custom.DownloadTask()
            }
            return eggy.cocos2dx.custom.DownloadTask.Companion.downloadUtil
        }
    }
}
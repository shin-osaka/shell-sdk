/****************************************************************************
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated engine source code (the "Software"), a limited,
 * worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 * to use Cocos Creator solely to develop games on your target platforms. You shall
 * not use Cocos Creator software for developing other software or tools that's
 * used for developing games. You are not granted to publish, distribute,
 * sublicense, and/or sell copies of Cocos Creator.
 *
 * The software or tools in this License Agreement are licensed, not sold.
 * Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.cocos2dx.lib

import android.R.attr.path
import okhttp3.Call
import okhttp3.Callback
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Request.Builder
import okhttp3.Response
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.PrintWriter
import java.net.URI
import java.net.URISyntaxException
import java.util.LinkedList
import java.util.Queue
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.TimeUnit


class Cocos2dxDownloader {
    private var _id = 0
    private var _httpClient: OkHttpClient? = null
    private var _tempFileNameSuffix: String? = null
    private var _countOfMaxProcessingTasks = 0
    private val _taskMap = ConcurrentHashMap<Int, Call>()
    private val _taskQueue: Queue<Runnable> = LinkedList()
    private var _runningTaskCount = 0
    private fun onProgress(id: Int, downloadBytes: Long, downloadNow: Long, downloadTotal: Long) {
        Cocos2dxHelper.runOnGLThread {
            nativeOnProgress(
                _id,
                id,
                downloadBytes,
                downloadNow,
                downloadTotal
            )
        }
    }

    private fun onFinish(id: Int, errCode: Int, errStr: String?, data: ByteArray?) {
         _taskMap[id] ?: return
        _taskMap.remove(id)
        _runningTaskCount -= 1
        Cocos2dxHelper.runOnGLThread { nativeOnFinish(_id, id, errCode, errStr, data) }
        runNextTaskIfExists()
    }

    private fun enqueueTask(taskRunnable: Runnable) {
        synchronized(_taskQueue) {
            if (_runningTaskCount < _countOfMaxProcessingTasks) {
                Cocos2dxHelper.activity!!.runOnUiThread(taskRunnable)
                _runningTaskCount++
            } else {
                _taskQueue.add(taskRunnable)
            }
        }
    }

    private fun runNextTaskIfExists() {
        synchronized(_taskQueue) {
            while (_runningTaskCount < _countOfMaxProcessingTasks &&
                _taskQueue.size > 0
            ) {
                val taskRunnable = _taskQueue.poll()
                Cocos2dxHelper.activity!!.runOnUiThread(taskRunnable)
                _runningTaskCount += 1
            }
        }
    }

    external fun nativeOnProgress(id: Int, taskId: Int, dl: Long, dlnow: Long, dltotal: Long)
    external fun nativeOnFinish(id: Int, taskId: Int,errCode: Int, errStr: String?, data: ByteArray?)

    companion object {
        private val _resumingSupport = ConcurrentHashMap<String?, Boolean>()

        @JvmStatic
        fun createDownloader(
            id: Int,
            timeoutInSeconds: Int,
            tempFileSuffix: String?,
            maxProcessingTasks: Int
        ): Cocos2dxDownloader {
            val downloader = Cocos2dxDownloader()
            downloader._id = id
            if (timeoutInSeconds > 0) {
                downloader._httpClient = OkHttpClient().newBuilder()
                    .followRedirects(true)
                    .followSslRedirects(true)
                    .callTimeout(timeoutInSeconds.toLong(), TimeUnit.SECONDS)
                    .build()
            } else {
                downloader._httpClient = OkHttpClient().newBuilder()
                    .followRedirects(true)
                    .followSslRedirects(true)
                    .build()
            }
            downloader._tempFileNameSuffix = tempFileSuffix
            downloader._countOfMaxProcessingTasks = maxProcessingTasks
            return downloader
        }

        @JvmStatic
        fun createTask(
            downloader: Cocos2dxDownloader,
            id_: Int,
            url_: String,
            path_: String,
            header_: Array<String>
        ) {
            val taskRunnable: Runnable = object : Runnable {
                var domain: String? = null
                var host: String? = null
                var tempFile: File? = null
                var finalFile: File? = null
                var downloadStart: Long = 0
                override fun run() {
                    var task: Call? = null
                    do {
                        if (path_.isNotEmpty()) {
                            domain = try {
                                val uri = URI(url_)
                                uri.host
                            } catch (e: URISyntaxException) {
                                e.printStackTrace()
                                break
                            } catch (e: NullPointerException) {
                                e.printStackTrace()
                                break
                            }
                            tempFile = File(path_ + downloader._tempFileNameSuffix)
                            if (tempFile!!.isDirectory) break
                            val parent = tempFile!!.parentFile
                            if (!parent.isDirectory && !parent.mkdirs()) break
                            finalFile = File(path_)
                            if (finalFile!!.isDirectory) break
                            val fileLen = tempFile!!.length()
                            host =
                                if (domain?.isNotEmpty()!! && domain?.startsWith("www.")!!) domain?.substring(
                                    4
                                ) else domain
                            if (fileLen > 0) {
                                if (_resumingSupport.containsKey(host) && _resumingSupport[host]!!) {
                                    downloadStart = fileLen
                                } else {
                                    try {
                                        val writer = PrintWriter(tempFile)
                                        writer.print("")
                                        writer.close()
                                    } catch (_: FileNotFoundException) {
                                    }
                                }
                            }
                        }
                        val builder: Builder = Builder().url(url_)
                        for (i in 0 until header_.size / 2) {
                            builder.addHeader(header_[i * 2], header_[i * 2 + 1])
                        }
                        if (downloadStart > 0) {
                            builder.addHeader("RANGE", "bytes=$downloadStart-")
                        }
                        val request: Request = builder.build()
                        task = downloader._httpClient!!.newCall(request)
                        if (task == null) {
                            val errStr = "Can't create DownloadTask for $url_"
                            Cocos2dxHelper.runOnGLThread {
                                downloader.nativeOnFinish(
                                    downloader._id,
                                    id_,
                                    0,
                                    errStr,
                                    null
                                )
                            }
                        } else {
                            downloader._taskMap[id_] = task
                        }
                        task.enqueue(object : Callback {
                            override fun onFailure(call: Call, e: IOException) {
                                downloader.onFinish(id_, 0, e.toString(), null)
                            }

                            @Throws(IOException::class)
                            override fun onResponse(call: Call, response: Response) {
                                var `is`: InputStream? = null
                                val buf = ByteArray(4096)
                                var fos: FileOutputStream? = null
                                try {
                                    if (response.code !in 200..206) {
                                        // it is encourage to delete the tmp file when requested range not satisfiable.
                                        // it is encourage to delete the tmp file when requested range not satisfiable.
                                        if (response.code == 416) {
                                            val file: File =
                                                File(path_ + downloader._tempFileNameSuffix)
                                            if (file.exists() && file.isFile) {
                                                file.delete()
                                            }
                                        }
                                        downloader.onFinish(id_, -2, response.message, null)
                                        return
                                    }
                                    val total = response.body!!.contentLength()
                                    if (path_.isNotEmpty() && !_resumingSupport.containsKey(
                                            host
                                        )
                                    ) {
                                        _resumingSupport[host!!] = total > 0
                                    }
                                    var current = downloadStart
                                    `is` = response.body!!.byteStream()
                                    if (path_.isNotEmpty()) {
                                        fos = if (downloadStart > 0) {
                                            FileOutputStream(tempFile, true)
                                        } else {
                                            FileOutputStream(tempFile, false)
                                        }
                                        var len: Int
                                        while (`is`.read(buf).also { len = it } != -1) {
                                            current += len.toLong()
                                            fos.write(buf, 0, len)
                                            downloader.onProgress(id_, len.toLong(), current, total)
                                        }
                                        fos.flush()
                                        var errStr: String? = null
                                        do {
                                            if (finalFile!!.exists()) {
                                                if (finalFile!!.isDirectory) {
                                                    break
                                                }
                                                if (!finalFile!!.delete()) {
                                                    errStr =
                                                        "Can't remove old file:" + finalFile!!.absolutePath
                                                    break
                                                }
                                            }
                                            tempFile!!.renameTo(finalFile)
                                        } while (false)
                                        if (errStr == null) {
                                            downloader.onFinish(id_, 0, null, null)
                                            downloader.runNextTaskIfExists()
                                        } else downloader.onFinish(id_, 0, errStr, null)
                                    } else {
                                        val buffer: ByteArrayOutputStream = if (total > 0) {
                                            ByteArrayOutputStream(total.toInt())
                                        } else {
                                            ByteArrayOutputStream(4096)
                                        }
                                        var len: Int
                                        while (`is`.read(buf).also { len = it } != -1) {
                                            current += len.toLong()
                                            buffer.write(buf, 0, len)
                                            downloader.onProgress(id_, len.toLong(), current, total)
                                        }
                                        downloader.onFinish(id_, 0, null, buffer.toByteArray())
                                        downloader.runNextTaskIfExists()
                                    }
                                } catch (e: IOException) {
                                    e.printStackTrace()
                                    downloader.onFinish(id_, 0, e.toString(), null)
                                } finally {
                                    try {
                                        `is`?.close()
                                        fos?.close()
                                    } catch (_: IOException) {
                                    }
                                }
                            }
                        })
                    } while (false)
                }
            }
            downloader.enqueueTask(taskRunnable)
        }

        @JvmStatic
        fun abort(downloader: Cocos2dxDownloader, id: Int) {
            Cocos2dxHelper.activity!!.runOnUiThread {
                val iter: Iterator<*> = downloader._taskMap.entries.iterator()
                while (iter.hasNext()) {
                    val (key1, value) = iter.next() as Map.Entry<*, *>
                    val key = key1!!
                    val task = value as Call?
                    if (null != task && key.toString().toInt() == id) {
                        task.cancel()
                        downloader._taskMap.remove(id)
                        downloader.runNextTaskIfExists()
                        break
                    }
                }
            }
        }

        @JvmStatic
        fun cancelAllRequests(downloader: Cocos2dxDownloader) {
            Cocos2dxHelper.activity!!.runOnUiThread {
                for (o in downloader._taskMap.entries) {
                    val (_, value) = o
                    val task = value as Call?
                    task?.cancel()
                }
            }
        }
    }
}
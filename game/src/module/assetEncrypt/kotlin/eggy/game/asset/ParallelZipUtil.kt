package eggy.game.asset

import eggy.util.LogUtil
import org.apache.commons.compress.archivers.zip.ParallelScatterZipCreator
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream
import org.apache.commons.compress.parallel.InputStreamSupplier
import java.io.File
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.OutputStream
import java.util.concurrent.ExecutionException
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit
import java.util.zip.Deflater

object ParallelZipUtil {
    /**
     * 压缩子文件集合
     *
     * @param zipOutput zip输出路径
     * @param inputDir  将要压缩的路径
     * @throws IOException
     * @throws ExecutionException
     * @throws InterruptedException
     */
    @Throws(IOException::class, ExecutionException::class, InterruptedException::class)
    fun compressFiles(inputDir: File, zipOutput: File?, includeSelf: Boolean) {
        val start = System.currentTimeMillis()
        //创建一个线程池对象
        val executor: ExecutorService = ThreadPoolExecutor(
            10,
            20,
            60,
            TimeUnit.SECONDS,
            LinkedBlockingQueue(20),
            Executors.defaultThreadFactory(),
            ThreadPoolExecutor.CallerRunsPolicy()
        )
        var files: Array<File>? = null
        files = if (includeSelf) {
            arrayOf(inputDir)
        } else {
            inputDir.listFiles()
        }

        //压缩等级默认为速度优先
        compressFiles(zipOutput, executor, Deflater.BEST_SPEED, files)
        val end = System.currentTimeMillis()
        LogUtil.i(
            ParallelZipUtil::class.java.simpleName,
            "parallex zip completed：%s ms",
            end - start
        )
    }

    /**
     * 自定义线程池
     *
     * @param zipOutput
     * @param executorService 线程池实现对象
     * @param paths
     * @throws IOException
     * @throws ExecutionException
     * @throws InterruptedException
     */
    @Throws(IOException::class, ExecutionException::class, InterruptedException::class)
    fun compressFiles(
        zipOutput: File?,
        executorService: ExecutorService?,
        level: Int,
        paths: Array<File>?
    ) {
        //      创建用于多线程压缩文件的对象
        val parallelScatterZipCreator = ParallelScatterZipCreator(executorService)
        //输出文件流
        val outputStream: OutputStream = FileOutputStream(zipOutput?.absolutePath)
        //输出Zip文件流
        val zipArchiveOutputStream = ZipArchiveOutputStream(outputStream)
        //设置压缩等级
        zipArchiveOutputStream.setLevel(level)
        //设置压缩的字符编码
        zipArchiveOutputStream.encoding = "UTF-8"
        //循环压缩各个路径的文件
        for (temp in paths!!) {
            compress(parallelScatterZipCreator, temp, temp.name)
        }
        //将数据写入zip输出流
        parallelScatterZipCreator.writeTo(zipArchiveOutputStream)
        //相关流的关闭
        zipArchiveOutputStream.close()
        outputStream.close()
    }

    /**
     * 遍历压缩
     *
     * @param parallelScatterZipCreator 线程池压缩对象
     * @param inputFile                 将要压缩的文件路径,绝对路径
     * @param relativePath              相对与压缩包内的路径
     * @throws IOException
     * @throws ExecutionException
     * @throws InterruptedException
     */
    @Throws(IOException::class, ExecutionException::class, InterruptedException::class)
    internal fun compress(
        parallelScatterZipCreator: ParallelScatterZipCreator,
        inputFile: File?,
        relativePath: String
    ) {
        //文件流为空，返回
        if (inputFile == null) {
            return
        }
        //文件为文件夹，递归遍历文件
        if (inputFile.isDirectory) {
            //    获取文件内的所有文件
            val files = inputFile.listFiles() ?: return
            //    遍历处理文件
            for (file in files) {
                if (file.isDirectory) {
                    compress(
                        parallelScatterZipCreator,
                        File(inputFile.absolutePath + "/" + file.name),
                        relativePath + "/" + file.name
                    )
                } else {
                    //    转化为InputStreamSupplier对象
                    val inputStreamSupplier = InputStreamSupplier {
                        try {
                            return@InputStreamSupplier FileInputStream(file)
                        } catch (e: FileNotFoundException) {
                            e.printStackTrace()
                            return@InputStreamSupplier null
                        }
                    }
                    //    添加ZipArchiveEntity对象，这里的构造函数的值，name属性，是相对于zip文件内的路径
                    val zipArchiveEntry = ZipArchiveEntry(relativePath + "/" + file.name)
                    //    设置压缩算法
                    zipArchiveEntry.method = ZipArchiveEntry.DEFLATED
                    //    设置未压缩文件的大小
                    zipArchiveEntry.size = file.length()
                    //    添加添加ZipArchiveEntity对象到多线程压缩中
                    parallelScatterZipCreator.addArchiveEntry(zipArchiveEntry, inputStreamSupplier)
                }
            }
        } else {
            //    当是文件时，直接处理
            val inputStreamSupplier = InputStreamSupplier {
                try {
                    return@InputStreamSupplier FileInputStream(inputFile)
                } catch (e: FileNotFoundException) {
                    e.printStackTrace()
                    return@InputStreamSupplier null
                }
            }
            val zipArchiveEntry = ZipArchiveEntry(inputFile.name)
            zipArchiveEntry.method = ZipArchiveEntry.DEFLATED
            zipArchiveEntry.size = inputFile.length()
            parallelScatterZipCreator.addArchiveEntry(zipArchiveEntry, inputStreamSupplier)
        }
    }
}
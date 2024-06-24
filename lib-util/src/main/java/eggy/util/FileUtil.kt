package eggy.util

import android.graphics.Bitmap
import java.io.BufferedInputStream
import java.io.BufferedOutputStream
import java.io.BufferedReader
import java.io.BufferedWriter
import java.io.File
import java.io.FileFilter
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.FilenameFilter
import java.io.IOException
import java.io.InputStream
import java.io.InputStreamReader
import java.io.OutputStream
import java.io.OutputStreamWriter
import java.nio.charset.StandardCharsets

object FileUtil {
    val TAG = FileUtil::class.java.simpleName
    private const val DEFAULT_BUFFER_SIZE = 8192
    fun deleteFileIfExists(file: File?) {
        if (file != null && file.exists()) {
            file.delete()
        }
    }

    fun ensureFile(file: File?) {
        if (file != null && !file.exists()) {
            ensureDir(file.parentFile)
            try {
                file.createNewFile()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }
    }

    fun ensureDir(directory: File?) {
        if (directory != null && !directory.exists()) {
            directory.mkdirs()
        }
    }

    @Throws(IOException::class)
    fun copyFile(src: InputStream, dst: String?) {
        if (!dst.isNullOrEmpty())
            copyFile(src, File(dst))
    }

    @Throws(IOException::class)
    fun copyFile(src: InputStream?, dst: File?): Boolean {
        var out: OutputStream? = null
        val success = try {
            deleteFile(dst)
            ensureFile(dst)
            out = BufferedOutputStream(FileOutputStream(dst))
            val copyBytes = src?.copyTo(out)!!
            (copyBytes > 0)
        } catch (e: Exception) {
            e.printStackTrace()
            false
        } finally {
            IOUtil.close(src)
            IOUtil.close(out)
        }
        return success
    }

    @Throws(IOException::class)
    fun copyFile(src: String?, dst: String?) {
        var `in`: BufferedInputStream? = null
        var ou: BufferedOutputStream? = null
        try {
            `in` = BufferedInputStream(FileInputStream(src))
            ou = BufferedOutputStream(FileOutputStream(dst))
            val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
            var read: Int
            while (`in`.read(buffer).also { read = it } != -1) {
                ou.write(buffer, 0, read)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(`in`)
            IOUtil.close(ou)
        }
    }


    fun saveBitmap(file: File?, bitmap: Bitmap) {
        var out: FileOutputStream? = null
        try {
            ensureFile(file)
            out = FileOutputStream(file)
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(out)
        }
    }

    fun readFile(file: File?): String? {
        var inputStream: InputStream? = null
        var streamReader: InputStreamReader? = null
        var bufferedReader: BufferedReader? = null
        try {
            inputStream = FileInputStream(file)
            streamReader = InputStreamReader(inputStream, StandardCharsets.UTF_8)
            bufferedReader = BufferedReader(streamReader)
            val builder = StringBuilder()
            var line: String?
            while (bufferedReader.readLine().also { line = it } != null) {
                builder.append(line)
            }
            return builder.toString()
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(inputStream)
            IOUtil.close(streamReader)
            IOUtil.close(bufferedReader)
        }
        return null
    }


    fun writeFile(file: File?, content: String?): Boolean {
        var outputStream: OutputStream? = null
        var streamWriter: OutputStreamWriter? = null
        var bufferedWriter: BufferedWriter? = null
        var success = false
        try {
            ensureFile(file)
            outputStream = FileOutputStream(file)
            streamWriter = OutputStreamWriter(outputStream, StandardCharsets.UTF_8)
            bufferedWriter = BufferedWriter(streamWriter)
            bufferedWriter.write(content)
            bufferedWriter.flush()
            success = true
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            IOUtil.close(outputStream)
            IOUtil.close(streamWriter)
            IOUtil.close(bufferedWriter)
        }
        return success
    }

    fun deleteFile(file: File?): Boolean {
        if (file == null) {
            return false
        }
        val files = file.listFiles()
        if (files != null && files.isNotEmpty()) {
            for (deleteFile in files) {
                if (deleteFile.isDirectory) {
                    if (!deleteFile(deleteFile)) {
                        return false
                    }
                } else {
                    if (!deleteFile.delete()) {
                        return false
                    }
                }
            }
        }
        return file.delete()
    }

    /**
     * Clean a specified directory.
     *
     * @param dir the directory to clean.
     */
    fun cleanDir(dir: File?) {
        deleteDir(dir, false)
    }

    /**
     * Clean a specified directory.
     *
     * @param dir    the directory to clean.
     * @param filter the filter to determine which file or directory to delete.
     */
    fun cleanDir(dir: File?, filter: FilenameFilter?) {
        deleteDir(dir, false, filter)
    }
    /**
     * Delete a specified directory.
     *
     * @param dir       the directory to clean.
     * @param removeDir true to remove the `dir`.
     */
    /**
     * Delete a specified directory.
     *
     * @param dir the directory to clean.
     */
    @JvmOverloads
    fun deleteDir(dir: File?, removeDir: Boolean = true): Boolean {
        if (dir != null && dir.isDirectory) {
            val files = dir.listFiles()
            if (!files.isNullOrEmpty()) {
                for (file in files) {
                    if (file.isDirectory) {
                        if (!deleteDir(file, removeDir)) return false
                    } else {
                        if (!file.delete()) return false
                    }
                }
            }
            return if (removeDir) {
                dir.delete()
            } else true
        }
        return false
    }

    /**
     * Delete a specified directory.
     *
     * @param dir       the directory to clean.
     * @param removeDir true to remove the `dir`.
     * @param filter    the filter to determine which file or directory to delete.
     */
    fun deleteDir(dir: File?, removeDir: Boolean, filter: FileFilter?) {
        if (dir != null && dir.isDirectory) {
            val files = dir.listFiles(filter)
            if (files != null) {
                for (file in files) {
                    if (file.isDirectory) {
                        deleteDir(file, removeDir, filter)
                    } else {
                        file.delete()
                    }
                }
            }
            if (removeDir) {
                dir.delete()
            }
        }
    }

    /**
     * Delete a specified directory.
     *
     * @param dir       the directory to clean.
     * @param removeDir true to remove the `dir`.
     * @param filter    the filter to determine which file or directory to delete.
     */
    fun deleteDir(dir: File?, removeDir: Boolean, filter: FilenameFilter?) {
        if (dir != null && dir.isDirectory) {
            val files = dir.listFiles(filter)
            if (files != null) {
                for (file in files) {
                    if (file.isDirectory) {
                        deleteDir(file, removeDir, filter)
                    } else {
                        file.delete()
                    }
                }
            }
            if (removeDir) {
                dir.delete()
            }
        }
    }

    /**
     * copy file to another file
     *
     * @param srcFile
     * @param destFile
     */
    fun copyFile(srcFile: File?, destFile: File): Boolean {
        var success = false
        var fis: FileInputStream? = null
        var fos: FileOutputStream? = null
        val parent = destFile.parentFile
        if (parent !=null && !parent.exists()) {
            parent.mkdirs()
        }
        try {
            if (!destFile.exists()) {
                destFile.createNewFile()
            }
            fis = FileInputStream(srcFile)
            fos = FileOutputStream(destFile)
            val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
            var bytesRead: Int
            while (fis.read(buffer).also { bytesRead = it } > 0) {
                fos.write(buffer, 0, bytesRead)
            }
            success = true
        } catch (e: Exception) {
            e.printStackTrace()
            // Delete it if copy fail
            destFile.delete()
        } finally {
            IOUtil.close(fis)
            IOUtil.close(fos)
        }
        return success
    }

    /**
     * Move the file.
     *
     * @param srcFile  The source file.
     * @param destFile The destination file.
     * @return `true`: success<br></br>`false`: fail
     */
    fun moveFile(srcFile: File?, destFile: File?): Boolean {
        return copyOrMoveFile(srcFile, destFile, true)
    }

    /**
     * Move the directory.
     *
     * @param srcDir  The source directory.
     * @param destDir The destination directory.
     * @return `true`: success<br></br>`false`: fail
     */
    fun moveDir(
        srcDir: File?,
        destDir: File?
    ): Boolean {
        return copyOrMoveDir(srcDir, destDir, true)
    }

    /**
     * Copy the directory.
     *
     * @param srcDir  The source directory.
     * @param destDir The destination directory.
     * @return `true`: success<br></br>`false`: fail
     */
    fun copyDir(
        srcDir: File?,
        destDir: File?
    ): Boolean {
        return copyOrMoveDir(srcDir, destDir, false)
    }

    private fun copyOrMoveDir(srcDir: File?, destDir: File?, isMove: Boolean): Boolean {
        if (srcDir == null || destDir == null) return false
        // destDir's path locate in srcDir's path then return false
        val srcPath = srcDir.path + File.separator
        val destPath = destDir.path + File.separator
        if (destPath.contains(srcPath)) return false
        if (!srcDir.exists() || !srcDir.isDirectory) return false
        if (!createOrExistsDir(destDir)) return false
        val files = srcDir.listFiles()
        if (files != null && files.size > 0) {
            for (file in files) {
                val oneDestFile = File(destPath + file.name)
                if (file.isFile) {
                    if (!copyOrMoveFile(file, oneDestFile, isMove)) return false
                } else if (file.isDirectory) {
                    if (!copyOrMoveDir(file, oneDestFile, isMove)) return false
                }
            }
        }
        return !isMove || deleteDir(srcDir)
    }

    private fun copyOrMoveFile(srcFile: File?, destFile: File?, isMove: Boolean): Boolean {
        if (srcFile == null || destFile == null) return false
        // srcFile equals destFile then return false
        if (srcFile == destFile) return false
        // srcFile doesn't exist or isn't a file then return false
        if (!srcFile.exists() || !srcFile.isFile) return false
        if (destFile.exists() && !destFile.delete()) {
            return false
        }
        return if (!createOrExistsDir(destFile.parentFile)) false else try {
            IOUtil.writeFileFromIS(
                destFile,
                FileInputStream(srcFile),
                false
            ) && !(isMove && !deleteFileIfExist(srcFile))
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
            false
        }
    }

    /**
     * Create a directory if it doesn't exist, otherwise do nothing.
     *
     * @param file The file.
     * @return `true`: exists or creates successfully<br></br>`false`: otherwise
     */
    fun createOrExistsDir(file: File?): Boolean {
        return file != null && if (file.exists()) file.isDirectory else file.mkdirs()
    }

    fun deleteFileIfExist(file: File?): Boolean {
        if (file == null) {
            return false
        }
        return if (file.exists()) {
            file.delete()
        } else false
    }

    /**
     * Create a file if it doesn't exist, otherwise do nothing.
     *
     * @param file The file.
     * @return `true`: exists or creates successfully<br></br>`false`: otherwise
     */
    fun createOrExistsFile(file: File?): Boolean {
        if (file == null) return false
        if (file.exists()) return file.isFile
        return if (!createOrExistsDir(file.parentFile)) false else try {
            file.createNewFile()
        } catch (e: IOException) {
            e.printStackTrace()
            false
        }
    }
}
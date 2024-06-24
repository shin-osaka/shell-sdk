package eggy.cocos2dx.lib

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.text.TextUtils
import code.dex.encrypt.CryptoTool
import eggy.util.FileUtil
import eggy.util.LogUtil
import eggy.util.MD5
import eggy.util.ZipUtil
import java.io.File
import java.io.IOException
import java.util.Arrays

/**
 * @author DeMon
 * Created on 8/5/23.
 * E-mail demonl@binarywalk.com
 * Desc:
 */
object CocosSoHelper {
    private const val TAG = "CocosSoHelper"
    private var mIsLoadSo = false

    /**
     * 动态加载Cocos so
     *
     * @param context
     */
    fun onLoadNativeLibraries(context: Context) {
        if (mIsLoadSo) return
        if (BuildConfig.SO_ENCRYPT_ENABLE) {
            val assetsSoName = getAssetsSoName(context)
            if (!TextUtils.isEmpty(assetsSoName)) {
                LogUtil.i(TAG, "onLoadNativeLibraries: start loading assets/%s", assetsSoName)
                val soFile = File(context.filesDir, "soLib/cocos_so_en.zip")
                //MD5校验，防止重复解密解压操作
                val isSame = checkSoMd5(context, assetsSoName, soFile)
                if (!isSame) {
                    var isSuccess = copyAssetsFile(context, assetsSoName, soFile)
                    if (isSuccess) {
                        LogUtil.i(
                            TAG, "onLoadNativeLibraries: copy assets success: assets/%s -> %s",
                            assetsSoName, soFile.absolutePath
                        )
                        val soZipFile = File(context.filesDir, "soLib/cocos_so.zip")
                        isSuccess = CryptoTool.decrypt(soFile, soZipFile)
                        if (isSuccess) {
                            LogUtil.i(
                                TAG, "onLoadNativeLibraries: decrypt so success: %s -> %s",
                                soFile.absolutePath, soZipFile.absolutePath
                            )
                            //解压so文件
                            val unzipDir = File(context.filesDir, "/soLib")
                            isSuccess = unzipJniLibs(soZipFile, unzipDir)
                            LogUtil.i(
                                TAG, "onLoadNativeLibraries: unzip so %s: %s -> %s",
                                if (isSuccess) "success" else "fail", soZipFile.absolutePath,
                                unzipDir.absolutePath
                            )
                            soZipFile.delete()
                        } else {
                            LogUtil.i(
                                TAG, "onLoadNativeLibraries: decrypt so fail: %s -> %s",
                                soFile.absolutePath, soZipFile.absolutePath
                            )
                        }
                    } else {
                        LogUtil.i(
                            TAG, "onLoadNativeLibraries: copy assets fail: %s -> %s",
                            assetsSoName, soFile.absolutePath
                        )
                    }
                } else {
                    LogUtil.i(
                        TAG,
                        "onLoadNativeLibraries: same so file exits, don't need to copy from assets: %s",
                        soFile.absolutePath
                    )
                }
            } else {
                LogUtil.i(TAG, "onLoadNativeLibraries: could not find so in assets")
            }
        } else {
            LogUtil.i(TAG, "onLoadNativeLibraries: skip loading assets because so encrypt disable")
        }
        loadSoLibrary(context)
    }

    private fun unzipJniLibs(soZipFile: File, unzipDir: File): Boolean {
        return ZipUtil.unZip(soZipFile, unzipDir, false)
    }

    private fun getSoPath(context: Context): String {
        val soFile: File = if (isSupportCpu64) {
            File(context.filesDir, "soLib/arm64-v8a/libeggygame.so")
        } else {
            File(context.filesDir, "soLib/armeabi-v7a/libeggygame.so")
        }
        return if (soFile.exists()) soFile.absolutePath else ""
    }

    private fun loadSoLibrary(context: Context) {
        try {
            val soPath = getSoPath(context)
            if (TextUtils.isEmpty(soPath) || !BuildConfig.SO_ENCRYPT_ENABLE) {
                val packagename = context.applicationInfo.packageName
                val ai = context.packageManager.getApplicationInfo(
                    packagename,
                    PackageManager.GET_META_DATA
                )
                val bundle = ai.metaData
                val libName = bundle.getString("android.app.lib_name")
                System.loadLibrary(libName)
                LogUtil.d(TAG, "onLoadNativeLibraries: load so success: $libName")
            } else {
                System.load(soPath)
                LogUtil.d(TAG, "onLoadNativeLibraries: load so success: $soPath")
            }
            mIsLoadSo = true
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "onLoadNativeLibraries: load so fail")
        }
    }

    private fun checkSoMd5(context: Context, assetsSoName: String?, soFile: File): Boolean {
        var isSame = false
        try {
            context.applicationContext.assets.open(assetsSoName!!).use { inputStream ->
                val assetMd5 = MD5.encrypt(inputStream)
                val soMd5 = MD5.encrypt(soFile)
                if (!TextUtils.isEmpty(assetMd5) && !TextUtils.isEmpty(soMd5)) {
                    isSame = assetMd5.equals(soMd5, ignoreCase = true)
                }
                LogUtil.d(
                    TAG,
                    "onLoadNativeLibraries: [checkMD5] assetsMd5=%s, fileMd5=%s, isSame=%s",
                    assetMd5,
                    soMd5,
                    isSame
                )
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return isSame
    }

    private fun copyAssetsFile(context: Context, assetsSoName: String, destFile: File): Boolean {
        var success = false
        try {
            val inputStream = context.applicationContext.assets.open(assetsSoName)
            success = FileUtil.copyFile(inputStream, destFile)
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return success
    }

    private fun getAssetsSoName(context: Context): String {
        try {
            val listName = context.applicationContext.assets.list("")
            if (listName != null) {
                LogUtil.d(
                    TAG,
                    "onLoadNativeLibraries: files in assets: %s",
                    Arrays.toString(listName)
                )
                for (name in listName) {
                    if (name.startsWith("cos") && name.endsWith(".data")) {
                        return name
                    }
                }
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return ""
    }

    val isSupportCpu64: Boolean
        /**
         * 当前手机是否是支持64位cpu
         *
         * @return
         */
        get() {
            var flag = false
            try {
                flag = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    val abis = Build.SUPPORTED_ABIS
                    LogUtil.i(TAG, "isCpu64: SUPPORTED_ABIS=" + Arrays.toString(abis))
                    val stringList = Arrays.asList(*abis)
                    return stringList.contains("arm64-v8a")
                } else {
                    "arm64-v8a" == Build.CPU_ABI
                }
            } catch (e: Exception) {
                e.printStackTrace()
                LogUtil.e(TAG, e, "isCpu64: ")
            }
            return flag
        }
}
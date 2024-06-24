package eggy.game.asset

import android.content.Context
import android.content.res.AssetManager
import code.dex.encrypt.CryptoTool
import eggy.manager.CocosAssetsMgr
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import eggy.game.BuildConfig
import eggy.util.LogUtil
import eggy.util.FileUtil
import eggy.util.MD5
import eggy.util.ZipUtil
import eggy.util.SysUtil

object AssetsUtil {
    private val TAG = AssetsUtil::class.java.simpleName

    fun decryptAssets(context: Context) {
        LogUtil.d(TAG, "assets encrypted, start decrypting")

        GlobalScope.launch(Dispatchers.IO) {
            //apk源文件目录
            val apkFile = File(context.applicationInfo.sourceDir)
            //自定义目录
            val versionDir: File = context.getDir(
                context.packageName + "_" +
                        SysUtil.getAppVersionCode(context), Context.MODE_PRIVATE
            )
            val appDir = File(versionDir, "app-1")
            //解压到自定义目录
            val success = ZipUtil.unZip(apkFile, appDir, true)
            if (success) {
                //解密cocos资源
                val unzippedApkPath: String? = decryptUnzipAssetsSource(context, appDir)
                //添加assets资源到delegate
                withContext(Dispatchers.Main) {
                    loadResources(unzippedApkPath)
                    AssetsLoader.onAssetLoaded()
                }
            }
        }
    }

    private fun loadResources(dexPath: String?): Boolean {
        var success = false
        try {
            LogUtil.d(TAG, "loadResource from: $dexPath")
            val assetManager = AssetManager::class.java.newInstance()
            val addAssetPath =
                AssetManager::class.java.getMethod("addAssetPath", String::class.java)
            addAssetPath.invoke(assetManager, dexPath)
            CocosAssetsMgr.onAssetsLoaded(assetManager)
            success = true
            LogUtil.d(TAG, "loadResource success")
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "loadResource error")
        }
        return success
    }
    fun decryptUnzipAssetsSource(context: Context, appDir: File): String? {
        var unzippedApkFile: File? = null
        val assetsDir = File(appDir, "assets")
        return if (assetsDir.exists()) {
            val resSubPackStoragePath = BuildConfig.COCOS_DEFAULT_RESOURCE_DIR_NAME
            val cocosNativeAssetsDir = File(context.filesDir, resSubPackStoragePath)
            FileUtil.ensureDir(cocosNativeAssetsDir)
            if (assetsDir.isDirectory) {
                //检测是否存在相同的assets资源，若存在，不再执行后续解密保存操作
                val md5 = MD5.encryptDir(assetsDir)
                unzippedApkFile = getUnzipedApkFile(context, md5)
                if (unzippedApkFile.exists()) {
                    LogUtil.d(
                        TAG,
                        "skip unzip plugin.jar,because we get the same dir,we already did it before"
                    )
                    return unzippedApkFile.absolutePath
                }
                //清空目录
                FileUtil.cleanDir(unzippedApkFile.parentFile)
                FileUtil.cleanDir(cocosNativeAssetsDir)
                val filesInAssets = assetsDir.listFiles { file ->
                    file.name.startsWith("coa") && file.name.endsWith(".data")
                }
                if (filesInAssets != null) {
                    for (file in filesInAssets) {
                        //解密资源文件
                        val decrypted = CryptoTool.decrypt(file, file)
                        LogUtil.d(
                            TAG,
                            "[assets] decrypt assets: %s, success: %s",
                            file.absolutePath,
                            decrypted
                        )
                        if (decrypted) {
                            //解压资源文件
                            LogUtil.d(TAG, "[assets] unzip assets begin: %s", file.absolutePath)
                            val unzipSuccess = ZipUtil.unZip(file, assetsDir, false)
                            LogUtil.d(
                                TAG,
                                "[assets] unzip assets: %s, success: %s",
                                file.absolutePath,
                                unzipSuccess
                            )
                            if (unzipSuccess) {
                                //删除加密源文件
                                val deleteResult = file.delete()
                                LogUtil.d(
                                    TAG,
                                    "[assets] delete zip assets: %s, success: %s",
                                    file.absolutePath,
                                    deleteResult
                                )
                                val assets = assetsDir.listFiles()
                                for (asset in assets) {
                                    if (!asset.name.endsWith(".data")) {
                                        //复制assets到cocos可直接读取的目录
                                        val targetFileOrDir = File(cocosNativeAssetsDir, asset.name)
                                        var copySuccess = false
                                        copySuccess = if (asset.isDirectory) {
                                            FileUtil.copyDir(asset, targetFileOrDir)
                                        } else {
                                            FileUtil.copyFile(asset, targetFileOrDir)
                                        }
                                        LogUtil.d(
                                            TAG,
                                            "[assets] copy assets file: %s -> %s, success: %s",
                                            asset.absolutePath,
                                            targetFileOrDir.absolutePath,
                                            copySuccess
                                        )
                                    }
                                }
                            }
                        }
                    }
                }
            }
            try {
                //此处打包apk到沙盒，用于反射AssetsManager.addAssetsPath方法
                ParallelZipUtil.compressFiles(assetsDir, unzippedApkFile, true)
                LogUtil.d(TAG, "zip new apk：%s", unzippedApkFile!!.absolutePath)
            } catch (e: Exception) {
                throw RuntimeException(e)
            }
            unzippedApkFile.absolutePath
        } else {
            LogUtil.d(TAG, "assets dir not exists: " + appDir.absolutePath)
            null
        }
    }

    private fun getUnzipedApkFile(context: Context, md5: String): File {
        LogUtil.d(TAG, "assets md5:%s", md5)
        val baseApkName = String.format("%s-base.apk", md5)
        val sandboxAssetsDir = File(context.filesDir, "assets")
        FileUtil.ensureDir(sandboxAssetsDir)
        return File(sandboxAssetsDir, baseApkName)
    }
}
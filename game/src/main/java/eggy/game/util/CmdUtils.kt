package eggy.game.util

import android.content.Context
import android.content.pm.ActivityInfo
import android.net.Uri
import android.os.Bundle
import android.text.TextUtils
import eggy.cocos2dx.lib.Cocos2dxActivity
import eggy.game.activity.WActivity
import eggy.game.core.CocosBridge
import eggy.game.core.JniBridge
import eggy.game.manager.AdjustManager
import eggy.game.manager.PermissionListener
import eggy.game.manager.PermissionManager
import eggy.game.util.ToastUtil.showLongToast
import eggy.res.ResourceLoader
import eggy.util.AppGlobal
import eggy.util.LogUtil
import eggy.util.NetworkUtil
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.json.JSONObject
import java.io.File

object CmdUtils {

    val TAG: String = CmdUtils::class.java.simpleName
    fun execCommand(command: String) {
        val uri = Uri.parse(command)
        val scheme = uri.scheme
        val host = uri.host
        val query = uri.query
        val bundle = getBundle(query)
        when {
            !scheme.isNullOrEmpty() && scheme.startsWith("game-") -> {
                val url = command.replace("game-", "")
                LogUtil.d(TAG, "game = $url")
                GlobalScope.launch(Dispatchers.Main) { openBrowserActivity(url) }
            }

            TextUtils.equals("command", host) -> {
                val cmd = bundle.getString("cmd")
                when (cmd) {
                    "quit_game" -> {
                        AppUtils.exitGame()
                    }

                    "start_watch_network" -> {
                        AppUtils.evalString(
                            String.format(
                                "cc.onNativeVersion(%s)",
                                AppUtils.VERSION
                            )
                        )
                    }

                    "network_state_changed" -> {
                        onNetworkStateChanged()
                    }
                }
            }

            TextUtils.equals("hide_splash", host) -> {
                AppUtils.hideSplash()
            }

            TextUtils.equals("report_notification_track", host) -> {
                AppUtils.reportTrack()
            }

            TextUtils.equals("configInfo", host) -> {
                val configInfo = command.replace("local://configInfo?url=", "")
                GlobalScope.launch(Dispatchers.Main) { setConfigInfo(configInfo) }
            }
        }
    }

    fun execCommand(command: String?, map: Map<String, Any?>) {
        val uri = Uri.parse(command)
        val host = uri.host

        when (host) {
            "cache_promotion_image" -> {
                val url = map["url"]
                val type = map["type"]
                if (url is String && type is Int) {
                    cachePromotionImage(url, type)
                }
            }

            "get_image_url" -> {
                val url = map["url"]
                if (url is String) {
                    saveImage(url)
                }
            }

            "save_image_to_album_and_copy_link" -> {
                val url = map["url"]
                val type = map["type"]
                if (url is String && type is Int) {
                    saveImageToAlbumAndCopyLink(url, type)
                }
            }

            "exchange_orientation" -> {
                val orientation = map["orientation"]
                if (orientation is Int) {
                    setScreenOrientation(orientation)
                }
            }
        }
    }

    private fun getBundle(query: String?): Bundle {
        val bundle = Bundle()
        try {
            if (!query.isNullOrEmpty()) {
                val params =
                    query.split("&".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
                if (params.isNotEmpty()) {
                    for (kv in params) {
                        val data =
                            kv.split("=".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
                        if (data.size == 2) {
                            bundle.putString(data[0], data[1])
                        }
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return bundle
    }


    private fun openBrowserActivity(url: String?) {
        if (!url.isNullOrEmpty()) {
            val context: Context = AppGlobal.application
            JniBridge.launcherView(context, WActivity::class.java.name, WActivity.KEY_URL, url)
        }
    }

    private fun onNetworkStateChanged() {
        val context = AppGlobal.application
        val netWorkState = when {
            NetworkUtil.isConnected(context) -> {
                1
            }
            NetworkUtil.isConnecting(context) -> {
                2
            }
            else -> {
                3
            }
        }
        AppUtils.evalString(String.format("cc.netWorkStateChanged(%s)", netWorkState))
    }

    private fun setConfigInfo(configInfoJsonStr: String) {
        val context = AppGlobal.application
        LogUtil.d(TAG, "setConfigInfo: %s", configInfoJsonStr)
        if (TextUtils.isEmpty(configInfoJsonStr)) {
            return
        }
        try {
            val configInfoObj = JSONObject(configInfoJsonStr)
            val adjustConf = configInfoObj.optJSONObject("adjustConf")
            if (null != adjustConf) {
                val adjustAppId = adjustConf.optString("appId")
                if (!TextUtils.isEmpty(adjustAppId)) {
                    AdjustManager.init(context, adjustAppId)
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }

    }

    private fun cachePromotionImage(urlStr: String, type: Int) {
        val context = AppGlobal.application
        val bundleId = ApplicationUtil.getPackageName(context)
        val channel = ApplicationUtil.channel
        var urlImageName = urlStr.substring(urlStr.lastIndexOf("/") + 1)
        urlImageName = urlImageName.replace(".jpg", "")
        urlImageName = urlImageName.replace(".png", "")
        val imageFileName = String.format("%s_%s_%s.jpg", bundleId, channel, urlImageName)
        if (urlStr.isEmpty() || imageFileName.isEmpty()) {
            onCacheImageDone(false, "")
            return
        }

        val imageUrl =
            CocosBridge.getStringFromPreference(
                String.format(
                    "%s_%s_%s_url",
                    bundleId,
                    channel,
                    type
                )
            )
        if (!imageUrl.isNullOrEmpty() && imageUrl == urlStr) {
            onCacheImageDone(true, urlStr)
            return
        }
        CocosBridge.saveStringToPreference(
            String.format("%s_%s_%s_url", bundleId, channel, type),
            urlStr
        )
        CocosBridge.saveStringToPreference(
            String.format("%s_%s_%s", bundleId, channel, type),
            imageFileName
        )
        eggy.cocos2dx.custom.DownloadTask.get()?.downloadJpeg(
            context,
            urlStr,
            imageFileName,
            object : eggy.cocos2dx.custom.DownloadTask.OnDownloadListener {
                override fun onDownloadSuccess(saveFile: File?) {
                    onCacheImageDone(true, urlStr)
                    if (saveFile != null) {
                        LogUtil.d(TAG, saveFile.absolutePath)
                    }
                }

                override fun onDownloading(progress: Int) {}
                override fun onDownloadFailed() {
                    onCacheImageDone(false, urlStr)
                    CocosBridge.saveStringToPreference(
                        String.format("%s_%s_%s_url", bundleId, channel, type),
                        ""
                    )
                }
            })
    }

    private fun onCacheImageDone(isSuccess: Boolean, url: String) {
        AppUtils.evalString(
            String.format(
                "cc.onNativeCachePromotionImage(\"%s\", \"%s\")",
                isSuccess,
                url
            )
        )
    }

    private fun saveImage(src: String) {
        ImageHelper.downloadImage2Photo(src)
    }

    private fun setScreenOrientation(orientation: Int) {
        val activity = AppUtils.curActivity
        if (activity != null) {
            activity.requestedOrientation =
                if (orientation == 0) ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE else ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
        }
    }

    private fun saveImageToAlbumAndCopyLink(url: String, type: Int) {
        if (TextUtils.isEmpty(url)) {
            return
        }
        CocosBridge.copyText(url)
        if (type == 1) {
            generateQrImageSaveAlbum(1, url, 212, 468, 1032)
        } else if (type == 2) {
            generateQrImageSaveAlbum(2, url, 228, 245, 936)
        }
    }

    private fun generateQrImageSaveAlbum(type: Int, url: String, qrSize: Int, x: Int, y: Int) {
        val context = Cocos2dxActivity.context
        if (!PermissionManager.checkPermission(context, PermissionManager.STORAGE_PERMISSIONS)) {
            PermissionManager.requestPermission(
                context,
                PermissionManager.STORAGE_PERMISSIONS,
                object : PermissionListener {
                    override fun permissionGranted(permission: Array<String?>) {
                        generateQrImageSaveAlbum(type, url, qrSize, x, y)
                    }

                    override fun permissionDenied(permission: Array<String?>) {
                        ResourceLoader.strings.save_image_permission_tips.showLongToast()
                    }
                })
            return
        }
        MergeTask(url, type, qrSize, x, y).execute()
    }
}
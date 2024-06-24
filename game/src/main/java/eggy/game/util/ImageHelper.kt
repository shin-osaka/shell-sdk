package eggy.game.util

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.net.Uri
import android.util.Base64
import eggy.game.manager.PermissionListener
import eggy.game.manager.PermissionManager
import eggy.game.util.ToastUtil.showLongToast
import eggy.res.ResourceLoader
import eggy.util.AppGlobal
import java.io.File

object ImageHelper {
    val TAG = ImageHelper::class.java.simpleName

    fun base64ToDrawable(base64: String): Drawable {
        return BitmapDrawable(AppGlobal.application.resources, base64ToBitmap(base64))
    }

    fun base64ToBitmap(base64: String): Bitmap {
        val bytes = Base64.decode(base64, Base64.DEFAULT)
        return BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
    }
    fun downloadImage2Photo(url: String?) {
        val context = AppGlobal.application
        if (!PermissionManager.checkPermission(context, PermissionManager.STORAGE_PERMISSIONS)) {
            PermissionManager.requestPermission(
                context,
                PermissionManager.STORAGE_PERMISSIONS,
                object : PermissionListener {
                    override fun permissionGranted(permission: Array<String?>) {
                        downloadImage2Photo(url)
                    }

                    override fun permissionDenied(permission: Array<String?>) {
                        ResourceLoader.strings.save_image_permission_tips.showLongToast()
                    }
                })
            return
        }
        downloadImage(url)
    }

    private fun downloadImage(url: String?) {
        eggy.cocos2dx.custom.DownloadTask.get()?.download(url!!, "ogame", object : eggy.cocos2dx.custom.DownloadTask.OnDownloadListener {
            override fun onDownloadSuccess(saveFile: File?) {
                if (saveFile == null) {
                    ResourceLoader.strings.save_error_retry_pls.showLongToast()
                    return
                }
                val context: Context = AppGlobal.application
                try {
                    context.sendBroadcast(
                        Intent(
                            Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(
                                File(saveFile.path)
                            )
                        )
                    )
                } catch (e: Exception) {
                    e.printStackTrace()
                }
                ResourceLoader.strings.image_save_success.showLongToast()
            }

            override fun onDownloading(progress: Int) {}

            override fun onDownloadFailed() {
                ResourceLoader.strings.save_error_retry_pls.showLongToast()
            }

        })
    }
}
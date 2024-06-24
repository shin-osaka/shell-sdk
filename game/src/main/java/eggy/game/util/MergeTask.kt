package eggy.game.util

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.LayerDrawable
import android.net.Uri
import android.os.AsyncTask
import android.os.Environment
import android.view.Gravity
import eggy.game.core.CocosBridge
import eggy.game.util.ToastUtil.showLongToast
import eggy.qrcode.QRCodeUtil
import eggy.res.ResourceLoader
import eggy.util.AppGlobal
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

class MergeTask(
    private val url: String,
    private val type: Int,
    private val qrSize: Int,
    private val x: Int,
    private val y: Int
) : AsyncTask<Void?, Int?, String?>() {
    override fun doInBackground(vararg voids: Void?): String? {
        val context = AppGlobal.application
        val baseBitmap = getCachePromotionImage(type) ?: return ResourceLoader.strings.copy_link_done
        val qrBitmap = QRCodeUtil.createQRCodeBitmap(qrSize, url)
        val array = arrayOfNulls<BitmapDrawable>(2)
        array[0] = BitmapDrawable(context.resources, baseBitmap)
        array[1] = BitmapDrawable(context.resources, qrBitmap)
        val la = LayerDrawable(array)
        array[1]!!.gravity = Gravity.LEFT or Gravity.TOP
        la.setLayerInset(0, 0, 0, 0, 0)
        la.setLayerInset(1, x, y, 0, 0)
        val bitmap =
            Bitmap.createBitmap(la.intrinsicWidth, la.intrinsicHeight, Bitmap.Config.RGB_565)
        val canvas = Canvas(bitmap)
        la.setBounds(0, 0, la.intrinsicWidth, la.intrinsicHeight)
        la.draw(canvas)
        saveImage(bitmap)
        return ResourceLoader.strings.save_image_and_open_app
    }

    override fun onPostExecute(s: String?) {
        s?.showLongToast()
    }

    private fun getCachePromotionImage(type: Int): Bitmap? {
        val context = AppGlobal.application
        val bundleId = ApplicationUtil.getPackageName(context)
        val channel = ApplicationUtil.channel
        val imageName = CocosBridge.getStringFromPreference(String.format("%s_%s_%s", bundleId, channel, type))
        if (imageName != null) {
            val file = File(context.filesDir, imageName)
            var bitmap: Bitmap? = null
            if (file.exists()) {
                bitmap = BitmapFactory.decodeFile(file.absolutePath)
            }
            return bitmap
        }
        return null
    }

    private fun saveImage(bmp: Bitmap) {
        val fileName = System.currentTimeMillis().toString() + ".jpg"
        val path = Environment
            .getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
            .toString() + "/" + fileName
        val file = File(path) //new File(appDir, fileName);
        try {
            val fos = FileOutputStream(file)
            bmp.compress(Bitmap.CompressFormat.JPEG, 60, fos)
            fos.flush()
            fos.close()
            val uri = Uri.fromFile(file)
            val context: Context = AppGlobal.application
            context.sendBroadcast(Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri))
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

}
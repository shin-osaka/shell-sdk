package eggy.game.util

import android.widget.Toast
import eggy.util.AppGlobal
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch

object ToastUtil {

    private var sToast: Toast? = null
    fun String.showLongToast() {
        showLongToast(this)
    }

    fun String.showShortToast() {
        showShortToast(this)
    }

    fun showLongToast(format: String?, vararg args: Any?) {
        MainScope().launch {
            val toast = String.format(format!!, *args)
            if (sToast != null) {
                sToast!!.cancel()
            }
            sToast = Toast.makeText(AppGlobal.application, toast, Toast.LENGTH_LONG)
            sToast!!.show()
        }
    }

    fun showShortToast(format: String?, vararg args: Any?) {
        MainScope().launch {
            val toast = String.format(format!!, *args)
            if (sToast != null) {
                sToast!!.cancel()
            }
            sToast = Toast.makeText(AppGlobal.application, toast, Toast.LENGTH_SHORT)
            sToast!!.show()
        }
    }
}
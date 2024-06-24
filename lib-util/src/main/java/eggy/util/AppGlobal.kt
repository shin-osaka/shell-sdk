package eggy.util

import android.app.Application

object AppGlobal {
    val TAG = AppGlobal::class.java.simpleName
    private var sApp: Application? = null

    fun setApplication(app: Application) {
        sApp = app
    }

    val application: Application
        get() {
            return sApp!!
        }

//    val application: Application
//        get() {
//            if (sApp == null) {
//                try {
//                    sApp = Class.forName("android.app.ActivityThread")
//                        .getMethod("currentApplication")
//                        .invoke(null) as? Application
//                } catch (e: Exception) {
//                    e.printStackTrace()
//                }
//            }
//            return sApp!!
//        }
}
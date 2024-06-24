package eggy.cocos2dx.lib

import android.content.Context
import android.content.pm.PackageManager
import eggy.util.LogUtil

/**
 * @author DeMon
 * Created on 8/5/23.
 * E-mail demonl@binarywalk.com
 * Desc:
 */
object CocosSoHelper {
    private const val TAG = "CocosSoHelper"
    private var sNativeInited = false

    /**
     * 动态加载Cocos so
     *
     * @param context
     */
    fun onLoadNativeLibraries(context: Context) {
        if (sNativeInited) return
        //load library whether so encrypt enable anyway
        loadSoLibrary(context)
    }


    private fun loadSoLibrary(context: Context) {
        sNativeInited = try {
            val pn = context.applicationInfo.packageName
            val ai = context.packageManager.getApplicationInfo(pn, PackageManager.GET_META_DATA)
            val bundle = ai.metaData
            val libName = bundle.getString("android.app.lib_name")
            System.loadLibrary(libName!!)
            LogUtil.d(TAG, "onLoadNativeLibraries: load so success: $libName")
            true
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "onLoadNativeLibraries: load so fail")
            false
        }
    }

}

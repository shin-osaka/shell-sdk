package eggy.game.manager

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity
import eggy.util.FormatUtil
import com.tbruyelle.rxpermissions3.RxPermissions
import eggy.util.SysUtil

object PermissionManager {
    val TAG = PermissionManager::class.java.simpleName


    var STORAGE_PERMISSIONS: Array<String?> = arrayOf(
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.READ_EXTERNAL_STORAGE
    )

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    var NOTIFICATIONS_PERMISSIONS: Array<String?> = arrayOf(
        Manifest.permission.POST_NOTIFICATIONS
    )
    private val mRequestFlags = HashMap<String, Boolean>()
    private val mRequestTimes = HashMap<String, Long>()

    /**
     * @param context
     * @param permissions
     * @return
     */
    fun checkPermission(context: Context?, permissions: Array<String?>): Boolean {
        val activity = SysUtil.findActivity(context)
        if (activity is FragmentActivity) {
            val rxPermissions = RxPermissions(activity)
            for (permission in permissions) {
                // Android10 以上已经不能申请存储权限
                if (Manifest.permission.WRITE_EXTERNAL_STORAGE == permission) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        return true
                    }
                }
                if (!rxPermissions.isGranted(permission)) {
                    return false
                }
            }
        } else {
            for (permission in permissions) {
                // Android10 以上已经不能申请存储权限
                if (Manifest.permission.WRITE_EXTERNAL_STORAGE == permission) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        return true
                    }
                }
                if (ContextCompat.checkSelfPermission(context!!, permission!!) !=
                    PackageManager.PERMISSION_GRANTED
                ) {
                    return false
                }
            }
        }
        return true
    }

    fun requestPermission(
        context: Context?,
        permissions: Array<String?>,
        permissionListener: PermissionListener?
    ) {
        val cacheKey = getCacheKey(permissions)
        if (isRequesting(cacheKey)) {
            return
        }
        val lastRequestTime = getLastRequestTime(cacheKey)
        if (lastRequestTime > 0 && System.currentTimeMillis() - lastRequestTime < 1000) {
            return
        }
        val activity = SysUtil.findActivity(context) ?: return
        setRequesting(cacheKey, true)
        if (activity is FragmentActivity) {
            val rxPermissions = RxPermissions(activity)
            rxPermissions.request(*permissions).subscribe { granted: Boolean ->
                setLastRequestTime(cacheKey, System.currentTimeMillis())
                setRequesting(cacheKey, false)
                if (granted) {
                    permissionListener?.permissionGranted(permissions)
                } else {
                    permissionListener?.permissionDenied(permissions)
                }
            }
        } else {
            val requestCode = FormatUtil.parseInt(cacheKey)
            ActivityCompat.requestPermissions(activity, permissions, requestCode)
            setLastRequestTime(cacheKey, System.currentTimeMillis())
            setRequesting(cacheKey, false)
            //how to listen
        }
    }

    private fun isRequesting(key: String): Boolean {
        var isrequesting = mRequestFlags[key]
        if (null == isrequesting) {
            isrequesting = java.lang.Boolean.FALSE
        }
        return isrequesting!!
    }

    private fun setRequesting(key: String, requesting: Boolean) {
        mRequestFlags[key] = requesting
    }

    private fun getLastRequestTime(key: String): Long {
        var lastRequestTime = mRequestTimes[key]
        if (lastRequestTime == null) {
            lastRequestTime = 0L
        }
        return lastRequestTime
    }

    private fun setLastRequestTime(key: String, requestTime: Long) {
        mRequestTimes[key] = requestTime
    }

    private fun getCacheKey(permissions: Array<String?>): String {
        val builder = StringBuilder()
        for (i in permissions.indices) {
            builder.append(permissions[i])
            if (i != permissions.size - 1) {
                builder.append(",")
            }
        }
        val hashCode = builder.toString().hashCode()
        return hashCode.toString()
    }
}
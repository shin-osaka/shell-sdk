package eggy.util

import android.app.Activity
import android.app.ActivityManager
import android.app.ActivityManager.RunningAppProcessInfo
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.ContextWrapper
import android.content.DialogInterface
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.database.Cursor
import android.graphics.Bitmap
import android.graphics.Rect
import android.graphics.drawable.BitmapDrawable
import android.net.Uri
import android.os.Build
import android.text.TextUtils
import java.net.NetworkInterface
import java.util.Collections
import java.util.Locale

object SysUtil {
    val TAG = SysUtil::class.java.simpleName

    /**
     * 获取软件版本名称
     */
    fun getAppVersion(context: Context?): String {
        var version = ""
        try {
            val packageInfo = context?.packageManager?.getPackageInfo(context.packageName, 0)
            if (packageInfo != null) {
                version = packageInfo.versionName
            }
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
        }
        return version
    }

    /**
     * 获取软件版本号
     */
    fun getAppVersionCode(context: Context?): Int {
        var versionCode = 0
        try {
            val packageInfo = context?.packageManager?.getPackageInfo(context.packageName, 0)
            if (packageInfo != null) {
                versionCode = packageInfo.versionCode
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return versionCode
    }

    /**
     * 获取自己应用程序的名称
     */
    fun getAppName(context: Context): String {
        var packageManager: PackageManager? = null
        var applicationInfo: ApplicationInfo?
        try {
            packageManager = context.packageManager
            applicationInfo = packageManager.getApplicationInfo(context.packageName, 0)
        } catch (var4: PackageManager.NameNotFoundException) {
            applicationInfo = null
        }
        return packageManager!!.getApplicationLabel(applicationInfo!!) as String
    }

    /**
     * 获取自己应用程序的图标
     */
    fun getAppIcon(context: Context): Bitmap {
        var packageManager: PackageManager? = null
        var applicationInfo: ApplicationInfo?
        try {
            packageManager = context.packageManager
            applicationInfo = packageManager.getApplicationInfo(context.packageName, 0)
        } catch (e: PackageManager.NameNotFoundException) {
            applicationInfo = null
        }
        val d = packageManager!!.getApplicationIcon(applicationInfo!!)
        val bd = d as BitmapDrawable
        return bd.bitmap
    }

    /**
     * 打开系统浏览器
     */
    @Throws(Exception::class)
    fun openBrowser(context: Context, url: String?) {
        val intent = Intent()
        intent.action = Intent.ACTION_VIEW
        intent.data = Uri.parse(url)
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
        context.startActivity(intent)
    }

    /**
     * 判断本程序界面是否传入后台运行了
     */
    fun isAppBackground(context: Context): Boolean {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager?
        if (activityManager != null) {
            val appProcesses = activityManager.runningAppProcesses
            for (appProcess in appProcesses) {
                if (appProcess.processName == context.packageName) {
                    return appProcess.importance == RunningAppProcessInfo.IMPORTANCE_BACKGROUND
                }
            }
        }
        return false
    }

    @OptIn(ExperimentalStdlibApi::class)
    var gsfAndroidId: String? = null
        get() {
            if (field.isNullOrEmpty()) {
                var cursor: Cursor? = null
                try {
                    val URI = Uri.parse("content://com.google.android.gsf.gservices")
                    val ID_KEY = "android_id"
                    val params = arrayOf(ID_KEY)
                    cursor = AppGlobal.application.contentResolver?.query(
                        URI,
                        null,
                        null,
                        params,
                        null
                    )
                    if (cursor != null && cursor.moveToFirst() && cursor.columnCount >= 2) {
                        val id = cursor.getString(1)
                        field = if (id.isNullOrEmpty() || "null" == id) null else id.toLong()
                            .toHexString()
                    }

                } catch (e: Exception) {
                    e.printStackTrace()
                } finally {
                    IOUtil.close(cursor)
                }
            }
            return field
        }
    /**
     * 获取状态栏高度，不能在Activity初始化使用。
     * 在onWindowFocusChanged中回调可以获取到正确高度
     */
    fun getStatusBarHeight(activity: Activity): Int {
        val rect = Rect()
        activity.window.decorView.getWindowVisibleDisplayFrame(rect)
        return rect.top
    }

    /**
     * 复制内容到剪贴板
     */
    fun copyToClipBoard(context: Context, label: String?, content: String?) {
        val manager = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager?
        manager?.setPrimaryClip(ClipData.newPlainText(label, content))
    }

    /**
     * 打开邮件客户端
     *
     * @param context
     * @param email
     */
    @Throws(Exception::class)
    fun openEmailClient(context: Context, email: String) {
        val emails = arrayOf(email) // 需要注意，email必须以数组形式传入
        val uri = Uri.parse("mailto:$email")
        val intent = Intent(Intent.ACTION_SENDTO, uri)
        intent.putExtra(Intent.EXTRA_EMAIL, emails) // 接收人
        val chooserIntent = Intent.createChooser(intent, "请选择邮件客户端")
        chooserIntent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
        context.startActivity(chooserIntent)
    }

    /**
     * 启动第三方APP
     *
     * @param context
     * @param packageName
     */
    @Throws(Exception::class)
    fun launchApp(context: Context, packageName: String?) {
        val packageManager = context.packageManager
        val intent = packageManager.getLaunchIntentForPackage(packageName!!)
        intent!!.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP
        context.startActivity(intent)
    }

    fun scanForActivity(context: Context?): Activity? {
        if (context == null) return null
        if (context is Activity) {
            return context
        } else if (context is ContextWrapper) {
            return scanForActivity(context.baseContext)
        }
        return null
    }

    /**
     * 判断某个界面是否在前台
     *
     * @param context   Context
     * @param className 界面的类名
     * @return 是否在前台显示
     */
    fun isForeground(context: Context?, cls: Class<*>?): Boolean {
        if (null == context || null == cls) {
            return false
        }
        try {
            val am = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
            val list = am.getRunningTasks(1)
            if (null == list || list.size == 0) {
                return false
            }
            val className = cls.name
            for (taskInfo in list) {
                if (null != taskInfo.topActivity) {
                    if (taskInfo.topActivity!!.shortClassName.contains(className)) { // 说明它已经启动了
                        return true
                    }
                }
            }
        } catch (_: Exception) {
        }
        return false
    }

    /**
     * 有時候dimiss对话框时Activity已销毁，会导致crash。
     *
     * @param dialog
     */
    fun dismissDialogSafety(dialog: DialogInterface?) {
        if (null != dialog) {
            try {
                dialog.dismiss()
            } catch (_: Exception) {
            }
        }
    }

    /**
     * 判断某一个类是否存在任务栈里面
     *
     * @return
     */
    fun isActivityRunning(context: Context, cls: Class<*>?): Boolean {
        var isRunning = false
        try {
            val intent = Intent(context, cls)
            val cmpName = intent.resolveActivity(context.packageManager)
            if (cmpName != null) { // 说明系统中存在这个activity
                val am = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
                val taskInfoList = am.getRunningTasks(10)
                for (taskInfo in taskInfoList) {
                    if (null != taskInfo.baseActivity && taskInfo.baseActivity == cmpName) { // 说明它已经启动了
                        isRunning = true
                        break
                    }
                }
            }
        } catch (_: Exception) {
        }
        return isRunning
    }

    fun isActivityDestroyed(activity: Activity?): Boolean {
        return null == activity || activity.isFinishing || Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 && activity.isDestroyed
    }

    fun getMemory(context: Context?, t: String?): String {
        var type = t
        if (TextUtils.isEmpty(type)) {
            type = "total"
        }
        try {
            val manager = context?.getSystemService(Activity.ACTIVITY_SERVICE) as ActivityManager?
            val info = ActivityManager.MemoryInfo()
            if (manager == null) {
                return "0.00"
            }
            manager.getMemoryInfo(info)
            return if (TextUtils.equals(type, "total")) {
                FormatUtil.formatSize(info.totalMem)
            } else {
                FormatUtil.formatSize(info.availMem)
            }
        } catch (_: Exception) {
        }
        return "0.00"
    }
    
    fun findActivity(context: Context?): Activity? {
        if (context is Activity) {
            return context
        }
        return if (context is ContextWrapper) {
            findActivity(context.baseContext)
        } else {
            null
        }
    }

    fun finishActivity(activity: Activity?) {
        try {
            activity?.finish()
        } catch (e: Exception) {
            LogUtil.e(TAG, e, "Fail to finish activity")
        }
    }
}
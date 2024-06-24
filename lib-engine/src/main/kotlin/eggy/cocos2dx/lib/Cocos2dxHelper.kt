/****************************************************************************
 * Copyright (c) 2010-2012 cocos2d-x.org
 * Copyright (c) 2013-2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos2d-x.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.cocos2dx.lib

import android.annotation.SuppressLint
import android.app.Activity
import android.content.BroadcastReceiver
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.media.AudioManager
import android.net.ConnectivityManager
import android.net.NetworkInfo
import android.net.Uri
import android.os.BatteryManager
import android.os.Build
import android.os.Environment
import android.os.Process
import android.os.Vibrator
import android.preference.PreferenceManager.OnActivityResultListener
import android.util.DisplayMetrics
import android.view.Surface
import android.view.WindowManager
import com.android.vending.expansion.zipfile.APKExpansionSupport
import com.android.vending.expansion.zipfile.ZipResourceFile
import eggy.manager.CocosAssetsMgr
import java.io.File
import java.io.IOException
import java.io.UnsupportedEncodingException
import java.lang.reflect.InvocationTargetException
import java.nio.charset.StandardCharsets
import java.util.Locale
import kotlin.math.max
import kotlin.math.min

@SuppressLint("StaticFieldLeak")
object Cocos2dxHelper {
    private const val PREFS_NAME = "Cocos2dxPrefsFile"
    private const val RUNNABLES_PER_FRAME = 5
    private val TAG = Cocos2dxHelper::class.java.simpleName
    var assetManager: AssetManager? = null
        private set
    private var sCocos2dxAccelerometer: Cocos2dxAccelerometer? = null
    private var sAccelerometerEnabled = false
    private const val sCompassEnabled = false

    @JvmStatic
    var isActivityVisible = false
        private set

    @JvmStatic
    var packageName: String? = null
        private set

    @JvmStatic
    var writablePath: String? = null
        private set

    @JvmStatic
    var activity: Activity? = null
        private set
    private var sCocos2dxHelperListener: Cocos2dxHelperListener? = null
    private val onActivityResultListeners: MutableSet<OnActivityResultListener> = LinkedHashSet()
    private var sVibrateService: Vibrator? = null
    private var sAssetsPath = ""
    var obbFile: ZipResourceFile? = null
        private set
    private val sBatteryReceiver = BatteryReceiver()

    @JvmStatic
    fun registerBatteryLevelReceiver(context: Context) {
        val intent =
            context.registerReceiver(sBatteryReceiver, IntentFilter(Intent.ACTION_BATTERY_CHANGED))
        sBatteryReceiver.setBatteryLevelByIntent(intent)
    }

    @JvmStatic
    fun unregisterBatteryLevelReceiver(context: Context) {
        context.unregisterReceiver(sBatteryReceiver)
    }

    const val NETWORK_TYPE_NONE = 0
    const val NETWORK_TYPE_LAN = 1
    const val NETWORK_TYPE_WWAN = 2

    @JvmStatic
    val networkType: Int
        get() {
            var status = NETWORK_TYPE_NONE
            val networkInfo: NetworkInfo?
            try {
                val connMgr =
                    activity?.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
                networkInfo = connMgr.activeNetworkInfo
            } catch (e: Exception) {
                e.printStackTrace()
                return status
            }
            if (networkInfo == null) {
                return status
            }
            val nType = networkInfo.type
            if (nType == ConnectivityManager.TYPE_MOBILE) {
                status = NETWORK_TYPE_WWAN
            } else if (nType == ConnectivityManager.TYPE_WIFI) {
                status = NETWORK_TYPE_LAN
            }
            return status
        }

    @JvmStatic
    fun runOnGLThread(r: Runnable?) {
        (activity as Cocos2dxActivity?)!!.runOnGLThread(r)
    }

    private var sInited = false

    @JvmStatic
    fun init(activity: Activity) {
        Cocos2dxHelper.activity = activity
        sCocos2dxHelperListener = activity as Cocos2dxHelperListener
        if (!sInited) {
            val applicationInfo = activity.applicationInfo
            packageName = applicationInfo.packageName
            CocosSoHelper.onLoadNativeLibraries(activity.applicationContext)
            val pm = activity.packageManager
            val isSupportLowLatency = pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY)
            var sampleRate = 44100
            var bufferSizeInFrames = 192
            if (Build.VERSION.SDK_INT >= 17) {
                val am = activity.getSystemService(Context.AUDIO_SERVICE) as AudioManager
                val audioManagerClass: Class<*> = AudioManager::class.java
                var parameters = arrayOf<Any?>(
                    Cocos2dxReflectionHelper.getConstantValue<String>(
                        audioManagerClass,
                        "PROPERTY_OUTPUT_SAMPLE_RATE"
                    )
                )
                val strSampleRate = Cocos2dxReflectionHelper.invokeInstanceMethod<String>(
                    am, "getProperty", arrayOf(
                        String::class.java
                    ), parameters
                )
                parameters = arrayOf(
                    Cocos2dxReflectionHelper.getConstantValue<String>(
                        audioManagerClass,
                        "PROPERTY_OUTPUT_FRAMES_PER_BUFFER"
                    )
                )
                val strBufferSizeInFrames = Cocos2dxReflectionHelper.invokeInstanceMethod<String>(
                    am, "getProperty", arrayOf(
                        String::class.java
                    ), parameters
                )
                sampleRate = strSampleRate?.toInt() ?: 44100
                bufferSizeInFrames = strBufferSizeInFrames?.toInt() ?: 192
            }
            nativeSetAudioDeviceInfo(isSupportLowLatency, sampleRate, bufferSizeInFrames)
            writablePath = activity.filesDir.absolutePath
            nativeSetApkPath(assetsPath)
            sCocos2dxAccelerometer = Cocos2dxAccelerometer(activity)
            assetManager = CocosAssetsMgr.getAssets(activity)
            nativeSetContext(activity as Context, assetManager)
            sVibrateService = activity.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
            sInited = true
            var versionCode = 1
            try {
                versionCode = Cocos2dxActivity.context.packageManager.getPackageInfo(
                    packageName!!, 0
                ).versionCode
            } catch (e: PackageManager.NameNotFoundException) {
                e.printStackTrace()
            }
            try {
                obbFile = APKExpansionSupport.getAPKExpansionZipFile(
                    Cocos2dxActivity.context,
                    versionCode,
                    0
                )
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }
    }

    @JvmStatic
    val assetsPath: String
        get() {
            if (sAssetsPath === "") {
                var versionCode = 1
                try {
                    versionCode =
                        activity!!.packageManager.getPackageInfo(packageName!!, 0).versionCode
                } catch (e: PackageManager.NameNotFoundException) {
                    e.printStackTrace()
                }
                val pathToOBB =
                    Environment.getExternalStorageDirectory().absolutePath + "/Android/obb/" + packageName + "/main." + versionCode + "." + packageName + ".obb"
                val obbFile = File(pathToOBB)
                if (obbFile.exists()) sAssetsPath = pathToOBB else sAssetsPath =
                    activity!!.applicationInfo.sourceDir
            }
            return sAssetsPath
        }

    @JvmStatic
    fun addOnActivityResultListener(listener: OnActivityResultListener) {
        onActivityResultListeners.add(listener)
    }

    @JvmStatic
    fun getOnActivityResultListeners(): Set<OnActivityResultListener> {
        return onActivityResultListeners
    }

    @JvmStatic
    private external fun nativeSetApkPath(pApkPath: String)

    @JvmStatic
    private external fun nativeSetEditTextDialogResult(pBytes: ByteArray)

    @JvmStatic
    private external fun nativeSetContext(pContext: Context, pAssetManager: AssetManager?)

    @JvmStatic
    private external fun nativeSetAudioDeviceInfo(
        isSupportLowLatency: Boolean,
        deviceSampleRate: Int,
        audioBufferSizeInFames: Int
    )

    @JvmStatic
    val currentLanguage: String
        get() = Locale.getDefault().language

    @JvmStatic
    val currentLanguageCode: String
        get() = Locale.getDefault().toString()

    @JvmStatic
    val deviceModel: String
        get() = Build.MODEL

    @JvmStatic
    fun enableAccelerometer() {
        sAccelerometerEnabled = true
        sCocos2dxAccelerometer!!.enable()
    }

    @JvmStatic
    fun setAccelerometerInterval(interval: Float) {
        sCocos2dxAccelerometer!!.setInterval(interval)
    }

    @JvmStatic
    fun disableAccelerometer() {
        sAccelerometerEnabled = false
        sCocos2dxAccelerometer!!.disable()
    }

    @JvmStatic
    fun setKeepScreenOn(value: Boolean) {
        (activity as Cocos2dxActivity?)!!.setKeepScreenOn(value)
    }

    @JvmStatic
    fun vibrate(duration: Float) {
        try {
            if (sVibrateService != null && sVibrateService!!.hasVibrator()) {
                if (Build.VERSION.SDK_INT >= 26) {
//                    val vibrationEffectClass = Class.forName("android.os.VibrationEffect")
//                    if (vibrationEffectClass != null) {
//                        val DEFAULT_AMPLITUDE = Cocos2dxReflectionHelper.getConstantValue<Int>(
//                            vibrationEffectClass,
//                            "DEFAULT_AMPLITUDE"
//                        )
//                        val method = vibrationEffectClass.getMethod(
//                            "createOneShot",
//                            java.lang.Long.TYPE, Integer.TYPE
//                        )
//                        val type = method.returnType
//                        val effect = method.invoke(
//                            vibrationEffectClass,
//                            (duration * 1000).toLong(),
//                            DEFAULT_AMPLITUDE
//                        )
//                        Cocos2dxReflectionHelper.invokeInstanceMethod<Any>(
//                            sVibrateService!!, "vibrate", arrayOf(type), arrayOf(
//                                effect
//                            )
//                        )
//                    }
                } else {
                    sVibrateService!!.vibrate((duration * 1000).toLong())
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    val version: String
        get() = try {
            Cocos2dxActivity.context.packageManager.getPackageInfo(
                Cocos2dxActivity.context.packageName,
                0
            ).versionName
        } catch (e: Exception) {
            ""
        }

    @JvmStatic
    fun openURL(url: String?): Boolean {
        var ret = false
        try {
            val i = Intent(Intent.ACTION_VIEW)
            i.data = Uri.parse(url)
            activity!!.startActivity(i)
            ret = true
        } catch (e: Exception) {
        }
        return ret
    }

    @JvmStatic
    fun copyTextToClipboard(text: String?) {
        activity!!.runOnUiThread {
            val myClipboard =
                activity!!.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            val myClip = ClipData.newPlainText("text", text)
            myClipboard.setPrimaryClip(myClip)
        }
    }

    @JvmStatic
    fun getObbAssetFileDescriptor(path: String?): LongArray {
        val array = LongArray(3)
        if (obbFile != null) {
            val descriptor = obbFile!!.getAssetFileDescriptor(path)
            if (descriptor != null) {
                try {
                    val parcel = descriptor.parcelFileDescriptor
                    val method = parcel.javaClass.getMethod("getFd")
                    array[0] = (method.invoke(parcel) as Int).toLong()
                    array[1] = descriptor.startOffset
                    array[2] = descriptor.length
                } catch (e: NoSuchMethodException) {
                } catch (e: IllegalAccessException) {
                } catch (e: InvocationTargetException) {
                }
            }
        }
        return array
    }

    @JvmStatic
    fun endApplication() {
        if (activity != null) activity!!.finish()
    }

    @JvmStatic
    fun onResume() {
        isActivityVisible = true
        if (sAccelerometerEnabled) {
            sCocos2dxAccelerometer!!.enable()
        }
    }

    @JvmStatic
    fun onPause() {
        isActivityVisible = false
        if (sAccelerometerEnabled) {
            sCocos2dxAccelerometer!!.disable()
        }
    }

    @JvmStatic
    fun onEnterBackground() {
    }

    @JvmStatic
    fun onEnterForeground() {
    }

    @JvmStatic
    fun terminateProcess() {
        Process.killProcess(Process.myPid())
    }

    @JvmStatic
    private fun showDialog(pTitle: String, pMessage: String) {
        sCocos2dxHelperListener!!.showDialog(pTitle, pMessage)
    }

    @JvmStatic
    fun setEditTextDialogResult(pResult: String) {
        val bytesUTF8 = pResult.toByteArray(StandardCharsets.UTF_8)
        sCocos2dxHelperListener!!.runOnGLThread { nativeSetEditTextDialogResult(bytesUTF8) }
    }

    @JvmStatic
    val dPI: Int
        get() {
            if (activity != null) {
                val metrics = DisplayMetrics()
                val wm = activity!!.windowManager
                if (wm != null) {
                    val d = wm.defaultDisplay
                    if (d != null) {
                        d.getMetrics(metrics)
                        return (metrics.density * 160.0f).toInt()
                    }
                }
            }
            return -1
        }

    @JvmStatic
    fun conversionEncoding(
        text: ByteArray?,
        fromCharset: String?,
        newCharset: String?
    ): ByteArray? {
        try {
            val str = String(text!!, charset(fromCharset!!))
            return str.toByteArray(charset(newCharset!!))
        } catch (e: UnsupportedEncodingException) {
            e.printStackTrace()
        }
        return null
    }

    @JvmStatic
    private fun setGameInfoDebugViewText(index: Int, text: String) {
        if (onGameInfoUpdatedListener != null) {
            when (index) {
                0 -> onGameInfoUpdatedListener!!.onGameInfoUpdated_0(text)
                1 -> onGameInfoUpdatedListener!!.onGameInfoUpdated_1(text)
                2 -> onGameInfoUpdatedListener!!.onGameInfoUpdated_2(text)
            }
        }
    }

    @JvmStatic
    private fun onJSExceptionCallback(location: String, message: String, stack: String) {
    }

    @JvmStatic
    private fun setJSBInvocationCount(count: Int) {
        if (onGameInfoUpdatedListener != null) {
            onGameInfoUpdatedListener!!.onJSBInvocationCountUpdated(count)
        }
    }

    @JvmStatic
    private fun openDebugView() {
        if (onGameInfoUpdatedListener != null) {
            onGameInfoUpdatedListener!!.onOpenDebugView()
        }
    }

    @JvmStatic
    private fun disableBatchGLCommandsToNative() {
        if (onGameInfoUpdatedListener != null) {
            onGameInfoUpdatedListener!!.onDisableBatchGLCommandsToNative()
        }
    }

    @JvmStatic
    var onGameInfoUpdatedListener: OnGameInfoUpdatedListener? = null

    private val sDeviceMotionValues = FloatArray(9)

    @JvmStatic
    private val deviceMotionValue: FloatArray
        get() {
            val event = sCocos2dxAccelerometer!!.deviceMotionEvent
            sDeviceMotionValues[0] = event.acceleration.x
            sDeviceMotionValues[1] = event.acceleration.y
            sDeviceMotionValues[2] = event.acceleration.z
            sDeviceMotionValues[3] = event.accelerationIncludingGravity.x
            sDeviceMotionValues[4] = event.accelerationIncludingGravity.y
            sDeviceMotionValues[5] = event.accelerationIncludingGravity.z
            sDeviceMotionValues[6] = event.rotationRate.alpha
            sDeviceMotionValues[7] = event.rotationRate.beta
            sDeviceMotionValues[8] = event.rotationRate.gamma
            return sDeviceMotionValues
        }

    @JvmStatic
    val sDKVersion: Int
        get() = Build.VERSION.SDK_INT

    @JvmStatic
    val systemVersion: String
        get() = Build.VERSION.RELEASE

    @JvmStatic
    val deviceRotation: Int
        get() {
            try {
                val display =
                    (activity!!.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay
                return display.rotation
            } catch (e: NullPointerException) {
                e.printStackTrace()
            }
            return Surface.ROTATION_0
        }

    @JvmStatic
    var batteryLevel = 0.0f
        private set

    /**
     * Battery receiver to getting battery level.
     */
    internal class BatteryReceiver : BroadcastReceiver() {

        override fun onReceive(context: Context, intent: Intent) {
            setBatteryLevelByIntent(intent)
        }

        fun setBatteryLevelByIntent(intent: Intent?) {
            if (null != intent) {
                val current = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0)
                val total = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 1)
                val level = current * 1.0f / total
                batteryLevel = min(max(level, 0.0f), 1.0f)
            }
        }
    }

    interface OnGameInfoUpdatedListener {
        fun onFPSUpdated(fps: Float)
        fun onJSBInvocationCountUpdated(count: Int)
        fun onOpenDebugView()
        fun onDisableBatchGLCommandsToNative()
        fun onGameInfoUpdated_0(text: String?)
        fun onGameInfoUpdated_1(text: String?)
        fun onGameInfoUpdated_2(text: String?)
    }

    interface Cocos2dxHelperListener {
        fun showDialog(pTitle: String?, pMessage: String?)
        fun runOnGLThread(pRunnable: Runnable?)
    }
}
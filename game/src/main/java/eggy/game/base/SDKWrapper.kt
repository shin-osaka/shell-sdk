/****************************************************************************
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated engine source code (the "Software"), a limited,
 * worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 * to use Cocos Creator solely to develop games on your target platforms. You shall
 * not use Cocos Creator software for developing other software or tools that's
 * used for developing games. You are not granted to publish, distribute,
 * sublicense, and/or sell copies of Cocos Creator.
 *
 * The software or tools in this License Agreement are licensed, not sold.
 * Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.game.base

import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.os.Bundle
import eggy.game.core.JniBridge
import eggy.manager.CocosAssetsMgr
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

@SuppressLint("StaticFieldLeak")
object SDKWrapper {
    val TAG = SDKWrapper::class.java.simpleName
    var context: Context? = null
        private set
    private var sdkClasses: List<SDKClass> = ArrayList()
    fun init(context: Context?) {
        this.context = context
        for (sdk in sdkClasses) {
            sdk.init(context)
        }
    }

    fun loadSDKClass() {
        val classes = ArrayList<SDKClass>()
        try {
            val json = getJson(context, "project.json")
            val sdks = JniBridge.instanceSDK(json)
            sdks?.forEach {
                if (it is SDKClass) {
                    classes.add(it)
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        sdkClasses = classes
    }

//    fun loadSDKClass() {
//        val classes = ArrayList<SDKClass>()
//        try {
//            val json = getJson(context, "project.json")
//            val jsonObject = JSONObject(json)
//            val serviceClassPath = jsonObject.optJSONArray("serviceClassPath") ?: return
//            val length = serviceClassPath.length()
//            for (i in 0 until length) {
//                val classPath = serviceClassPath.getString(i)
//                val sdk = Class.forName(classPath).newInstance() as SDKClass
//                classes.add(sdk)
//            }
//        } catch (e: Exception) {
//            e.printStackTrace()
//        }
//        sdkClasses = classes
//    }

    private fun getJson(mContext: Context?, fileName: String): String {
        val sb = StringBuilder()
        val am = CocosAssetsMgr.getAssets(mContext!!)
        try {
            val br = BufferedReader(InputStreamReader(am.open(fileName)))
            var next: String?
            while (null != br.readLine().also { next = it }) {
                sb.append(next)
            }
        } catch (e: IOException) {
            e.printStackTrace()
            sb.delete(0, sb.length)
        }
        return sb.toString().trim { it <= ' ' }
    }

    fun setGLSurfaceView(view: GLSurfaceView?, context: Context?) {
        this.context = context
        loadSDKClass()
        for (sdk in sdkClasses) {
            sdk.setGLSurfaceView(view)
        }
    }

    fun onResume() {
        for (sdk in sdkClasses) {
            sdk.onResume()
        }
    }

    fun onPause() {
        for (sdk in sdkClasses) {
            sdk.onPause()
        }
    }

    fun onDestroy() {
        for (sdk in sdkClasses) {
            sdk.onDestroy()
        }
    }

    fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        for (sdk in sdkClasses) {
            sdk.onActivityResult(requestCode, resultCode, data)
        }
    }

    fun onNewIntent(intent: Intent?) {
        for (sdk in sdkClasses) {
            sdk.onNewIntent(intent)
        }
    }

    fun onRestart() {
        for (sdk in sdkClasses) {
            sdk.onRestart()
        }
    }

    fun onStop() {
        for (sdk in sdkClasses) {
            sdk.onStop()
        }
    }

    fun onBackPressed() {
        for (sdk in sdkClasses) {
            sdk.onBackPressed()
        }
    }

    fun onConfigurationChanged(newConfig: Configuration?) {
        for (sdk in sdkClasses) {
            sdk.onConfigurationChanged(newConfig)
        }
    }

    fun onRestoreInstanceState(savedInstanceState: Bundle?) {
        for (sdk in sdkClasses) {
            sdk.onRestoreInstanceState(savedInstanceState)
        }
    }

    fun onSaveInstanceState(outState: Bundle?) {
        for (sdk in sdkClasses) {
            sdk.onSaveInstanceState(outState)
        }
    }

    fun onStart() {
        for (sdk in sdkClasses) {
            sdk.onStart()
        }
    }
}
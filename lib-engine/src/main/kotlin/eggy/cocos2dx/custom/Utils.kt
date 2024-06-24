/****************************************************************************
 * Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.
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
package eggy.cocos2dx.custom

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.net.ConnectivityManager
import android.os.Build
import android.view.View
import eggy.cocos2dx.lib.Cocos2dxReflectionHelper

@SuppressLint("StaticFieldLeak")
object Utils {
    private var sActivity: Activity? = null

    @JvmStatic
    fun setActivity(activity: Activity?) {
        sActivity = activity
    }

    @JvmStatic
    fun hideVirtualButton() {
        if (Build.VERSION.SDK_INT >= 19 && null != sActivity) {
            val viewClass: Class<*> = View::class.java
            val SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION =
                Cocos2dxReflectionHelper.getConstantValue<Int>(
                    viewClass,
                    "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION"
                )!!
            val SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = Cocos2dxReflectionHelper.getConstantValue<Int>(
                viewClass,
                "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN"
            )!!
            val SYSTEM_UI_FLAG_HIDE_NAVIGATION = Cocos2dxReflectionHelper.getConstantValue<Int>(
                viewClass,
                "SYSTEM_UI_FLAG_HIDE_NAVIGATION"
            )!!
            val SYSTEM_UI_FLAG_FULLSCREEN = Cocos2dxReflectionHelper.getConstantValue<Int>(
                viewClass,
                "SYSTEM_UI_FLAG_FULLSCREEN"
            )!!
            val SYSTEM_UI_FLAG_IMMERSIVE_STICKY = Cocos2dxReflectionHelper.getConstantValue<Int>(
                viewClass,
                "SYSTEM_UI_FLAG_IMMERSIVE_STICKY"
            )!!
            val SYSTEM_UI_FLAG_LAYOUT_STABLE = Cocos2dxReflectionHelper.getConstantValue<Int>(
                viewClass,
                "SYSTEM_UI_FLAG_LAYOUT_STABLE"
            )!!

            val parameters = arrayOf<Any?>(
                SYSTEM_UI_FLAG_LAYOUT_STABLE
                        or SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        or SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        or SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                        or SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                        or SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            )
            Cocos2dxReflectionHelper.invokeInstanceMethod<Void>(
                sActivity!!.window.decorView,
                "setSystemUiVisibility", arrayOf(Integer.TYPE),
                parameters
            )
        }
    }

    @JvmStatic
    fun isNetworkConnected(): Boolean {
        if (sActivity == null) {
            return false;
        }
        val cm = sActivity?.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val activeNetwork = cm.activeNetworkInfo
        return activeNetwork != null &&
                activeNetwork.isConnected
    }

}
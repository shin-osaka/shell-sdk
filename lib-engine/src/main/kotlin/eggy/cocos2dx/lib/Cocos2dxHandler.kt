/****************************************************************************
 * Copyright (c) 2010-2013 cocos2d-x.org
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

import android.app.AlertDialog
import android.os.Handler
import android.os.Looper
import android.os.Message
import java.lang.ref.WeakReference

class Cocos2dxHandler(activity: Cocos2dxActivity) : Handler(Looper.getMainLooper()) {
    private val mActivity: WeakReference<Cocos2dxActivity>

    init {
        mActivity = WeakReference(activity)
    }

    override fun handleMessage(msg: Message) {
        when (msg.what) {
            HANDLER_SHOW_DIALOG -> showDialog(msg)
        }
    }

    private fun showDialog(msg: Message) {
        val theActivity = mActivity.get()
        val dialogMessage = msg.obj as DialogMessage
        AlertDialog.Builder(theActivity)
            .setTitle(dialogMessage.title)
            .setMessage(dialogMessage.message)
            .setPositiveButton(
                "Ok"
            ) { _, _ -> }.create().show()
    }

    class DialogMessage(var title: String, var message: String)
    companion object {
        const val HANDLER_SHOW_DIALOG = 1
    }
}
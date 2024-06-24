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

import android.content.Context
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Point
import android.graphics.Rect
import android.graphics.drawable.Drawable
import android.graphics.drawable.ShapeDrawable
import android.graphics.drawable.StateListDrawable
import android.graphics.drawable.shapes.RoundRectShape
import android.text.Editable
import android.text.InputFilter
import android.text.InputFilter.LengthFilter
import android.text.InputType
import android.text.TextUtils
import android.text.TextWatcher
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputMethodManager
import android.widget.Button
import android.widget.EditText
import android.widget.RelativeLayout
import eggy.cocos2dx.custom.NumericUtils
import eggy.cocos2dx.custom.Utils
import eggy.util.LogUtil
import eggy.cocos2dx.lib.R
import eggy.res.ResourceLoader

class Cocos2dxEditBox(context: Cocos2dxActivity, layout: RelativeLayout) {
    private var mEditText: Cocos2dxEditText? = null
    private var mButton: Button? = null
    private var mButtonTitle: String? = null
    private var mConfirmHold = true
    private var mActivity: Cocos2dxActivity? = null
    private var mButtonLayout: RelativeLayout? = null
    private var mButtonParams: RelativeLayout.LayoutParams? = null
    private val mEditTextID = 1
    private val mButtonLayoutID = 2
    private var myLayout: RelativeLayout? = null
    private val mScreenHeight: Int
    private var mTopMargin = 0

    /***************************************************************************************
     * Inner class.
     */
    internal inner class Cocos2dxEditText(context: Cocos2dxActivity) :
        androidx.appcompat.widget.AppCompatEditText(context) {
        private val TAG = "Cocos2dxEditBox"
        private var mIsMultiLine = false
        private var mTextWatcher: TextWatcher? = null
        private val mLineColor = DARK_GREEN
        private val mLineWidth = 2f
        private var keyboardVisible = false
        private val mScreenHeight: Int
        private var canMoveCursor = true

        init {
            setBackgroundColor(Color.WHITE)
            mScreenHeight =
                (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.height
            mTextWatcher = object : TextWatcher {
                override fun beforeTextChanged(
                    text: CharSequence,
                    start: Int,
                    count: Int,
                    after: Int
                ) {
                }

                override fun onTextChanged(
                    text: CharSequence,
                    start: Int,
                    before: Int,
                    count: Int
                ) {
                    if (useThousandthFormat(sInputMode)) {
                        val amountValue = NumericUtils.transferToNumber(mEditText!!.text.toString())
                        if (NumericUtils.touzi_ed_values22 != amountValue) {
                            mEditText!!.setText(NumericUtils.addComma(amountValue, mEditText!!))
                        }
                    }
                }

                override fun afterTextChanged(text: Editable) {
                    val finalText = if (useThousandthFormat(sInputMode)) {
                        NumericUtils.transferToNumber(text.toString())
                    } else {
                        text.toString()
                    }
                    onKeyboardInput(finalText)
                }
            }
            registKeyboardVisible()
        }

        fun setCanMoveCursor(canMoveCursor: Boolean) {
            this.canMoveCursor = canMoveCursor
        }

        fun resetInputCache() {
            NumericUtils.touzi_ed_values22 = ""
        }

        override fun onSelectionChanged(selStart: Int, selEnd: Int) {
            super.onSelectionChanged(selStart, selEnd)
            if (!canMoveCursor) {
                if (selStart == selEnd) { //防止不能多选
                    val tt = text
                    if (tt.isNullOrEmpty()) { //判空，防止出现空指针
                        setSelection(0)
                    } else {
                        setSelection(tt.length) // 保证光标始终在最后面
                    }
                }
            }
        }
        /***************************************************************************************
         * Override functions.
         */
        /***************************************************************************************
         * Public functions.
         */
        fun show(
            defaultValue: String,
            maxLength: Int,
            isMultiline: Boolean,
            confirmHold: Boolean,
            confirmType: String,
            inputType: String
        ) {
            mIsMultiLine = isMultiline
            sInputMode = inputType
            this.filters = arrayOf<InputFilter>(LengthFilter(maxLength))
            if (useThousandthFormat(sInputMode)) {
                val amountValue = NumericUtils.transferToNumber(defaultValue)
                this.setText(NumericUtils.addComma(amountValue, mEditText!!))
            } else {
                this.setText(defaultValue)
            }
            val tt = this.text
            if (!tt.isNullOrEmpty()) {
                if (tt.length >= defaultValue.length) {
                    this.setSelection(defaultValue.length)
                } else {
                    this.setSelection(tt.length)
                }
            }
            setConfirmType(confirmType)
            this.setInputType(inputType, mIsMultiLine)
            this.visibility = VISIBLE
            this.requestFocus()
            addListeners()
        }

        fun hide() {
            mEditText!!.visibility = INVISIBLE
            removeListeners()
        }

        /***************************************************************************************
         * Private functions.
         */
        private fun setConfirmType(confirmType: String) {
            if (confirmType.contentEquals("done")) {
                this.imeOptions = EditorInfo.IME_ACTION_DONE or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                mButtonTitle = ResourceLoader.strings.done
            } else if (confirmType.contentEquals("next")) {
                this.imeOptions = EditorInfo.IME_ACTION_NEXT or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                mButtonTitle = ResourceLoader.strings.next
            } else if (confirmType.contentEquals("search")) {
                this.imeOptions = EditorInfo.IME_ACTION_SEARCH or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                mButtonTitle = ResourceLoader.strings.search
            } else if (confirmType.contentEquals("go")) {
                this.imeOptions = EditorInfo.IME_ACTION_GO or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                mButtonTitle = ResourceLoader.strings.go
            } else if (confirmType.contentEquals("send")) {
                this.imeOptions = EditorInfo.IME_ACTION_SEND or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                mButtonTitle = ResourceLoader.strings.send
            } else {
                mButtonTitle = null
            }
        }

        fun setInputType(inputType: String, isMultiLine: Boolean) {
            when {
                inputType.contentEquals("text") -> {
                    if (isMultiLine) {
                        this@Cocos2dxEditText.inputType =
                            InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE
                        this@Cocos2dxEditText.maxLines = 3
                        this@Cocos2dxEditText.maxHeight = dip2px(context, 50f)
                    } else {
                        this@Cocos2dxEditText.inputType = InputType.TYPE_CLASS_TEXT
                    }
                }

                inputType.contentEquals("email") -> this@Cocos2dxEditText.inputType =
                    InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS

                inputType.contentEquals("number") -> this@Cocos2dxEditText.inputType =
                    InputType.TYPE_CLASS_NUMBER

                inputType.contentEquals("decimal") -> this@Cocos2dxEditText.inputType =
                    InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_DECIMAL

                inputType.contentEquals("phone") -> this@Cocos2dxEditText.inputType =
                    InputType.TYPE_CLASS_PHONE

                inputType.contentEquals("password") -> this@Cocos2dxEditText.inputType =
                    InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD

                else -> LogUtil.e(TAG, "unknown input type $inputType")
            }
        }

        private fun addListeners() {
            setOnEditorActionListener { _, _, _ ->
                if (!mIsMultiLine) {
                    this@Cocos2dxEditBox.hide()
                }
                false // pass on to other listeners.
            }
            addTextChangedListener(mTextWatcher)
        }

        private fun removeListeners() {
            setOnEditorActionListener(null)
            removeTextChangedListener(mTextWatcher)
        }

        private fun registKeyboardVisible() {
            viewTreeObserver.addOnGlobalLayoutListener {
                val r = Rect()
                getWindowVisibleDisplayFrame(r)
                val heightDiff = rootView.height - (r.bottom - r.top)
                if (heightDiff > mScreenHeight / 4) {
                    if (!keyboardVisible) {
                        keyboardVisible = true
                        val layoutParams = RelativeLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                        )
                        if (mTopMargin > heightDiff) {
                            mTopMargin = heightDiff
                        }
                        var top = mScreenHeight - mTopMargin
                        if (top < 0) {
                            top = 0
                        }
                        layoutParams.topMargin = top
                        myLayout!!.layoutParams = layoutParams
                    }
                } else {
                    if (keyboardVisible) {
                        keyboardVisible = false
                        this@Cocos2dxEditBox.hide()
                    }
                }
            }
        }
    }

    /***************************************************************************************
     * Private functions.
     */
    private fun addItems(context: Cocos2dxActivity, layout: RelativeLayout) {
        myLayout = RelativeLayout(context)
        myLayout!!.setBackgroundColor(-0x1)
        val padding_left = dip2px(context, 50f)
        myLayout!!.setPadding(padding_left, 0, padding_left, 0)
        addEditText(context, myLayout!!)
        addButton(context, myLayout!!)
        val layoutParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM)
        myLayout!!.visibility = View.GONE
        layout.addView(myLayout, layoutParams)
    }

    private fun addEditText(context: Cocos2dxActivity, layout: RelativeLayout) {
        mEditText = Cocos2dxEditText(context)
        mEditText!!.textSize = 16f
        mEditText!!.visibility = View.INVISIBLE
        mEditText!!.id = mEditTextID
        mEditText!!.gravity = Gravity.CENTER
        val editParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT
        )
        editParams.addRule(RelativeLayout.LEFT_OF, mButtonLayoutID)
        layout.addView(mEditText, editParams)
    }

    private fun addButton(context: Cocos2dxActivity, layout: RelativeLayout) {
        mButton = Button(context)
        mButtonParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        mButtonParams!!.setMargins(5, 0, 0, 0)
        mButton!!.setTextColor(Color.WHITE)
        mButton!!.background = roundRectShape
        mButtonLayout = RelativeLayout(Cocos2dxHelper.activity)
        mButtonLayout!!.visibility = View.INVISIBLE
        mButtonLayout!!.setBackgroundColor(Color.WHITE)
        val buttonLayoutParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT
        )
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT)
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_BOTTOM, mEditTextID)
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_TOP, mEditTextID)
        mButtonLayout!!.addView(mButton, mButtonParams)
        mButtonLayout!!.id = mButtonLayoutID
        layout.addView(mButtonLayout, buttonLayoutParams)
        mButton!!.setOnClickListener {
            onKeyboardConfirm(mEditText!!.text.toString())
            if (!mConfirmHold) hide()
        }
    }

    private val roundRectShape: Drawable
        get() {
            val radius = 7
            val outerRadii = floatArrayOf(
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat(),
                radius.toFloat()
            )
            val roundRectShape = RoundRectShape(outerRadii, null, null)
            val shapeDrawableNormal = ShapeDrawable()
            shapeDrawableNormal.shape = roundRectShape
            shapeDrawableNormal.paint.style = Paint.Style.FILL
            shapeDrawableNormal.paint.color = DARK_GREEN
            val shapeDrawablePress = ShapeDrawable()
            shapeDrawablePress.shape = roundRectShape
            shapeDrawablePress.paint.style = Paint.Style.FILL
            shapeDrawablePress.paint.color = DARK_GREEN_PRESS
            val drawable = StateListDrawable()
            drawable.addState(intArrayOf(android.R.attr.state_pressed), shapeDrawablePress)
            drawable.addState(intArrayOf(), shapeDrawableNormal)
            return drawable
        }

    private fun hide() {
        Utils.hideVirtualButton()
        mEditText!!.hide()
        mButtonLayout!!.visibility = View.INVISIBLE
        myLayout!!.visibility = View.GONE
        closeKeyboard()
        mActivity!!.gLSurfaceView!!.requestFocus()
        mActivity!!.gLSurfaceView!!.setStopHandleTouchAndKeyEvents(false)
    }

    private fun show(
        defaultValue: String,
        maxLength: Int,
        isMultiline: Boolean,
        confirmHold: Boolean,
        confirmType: String,
        inputType: String,
        x: Int,
        yy: Int,
        width: Int,
        height: Int
    ) {
        var y = yy
        mConfirmHold = confirmHold
        mEditText!!.show(defaultValue, maxLength, isMultiline, confirmHold, confirmType, inputType)
        val editPaddingBottom = mEditText!!.paddingBottom
        val editPadding = mEditText!!.paddingTop
        mEditText!!.setPadding(editPadding, editPadding, editPadding, editPaddingBottom)
        mButton!!.text = mButtonTitle
        if (TextUtils.isEmpty(mButtonTitle)) {
            mButton!!.setPadding(0, 0, 0, 0)
            mButtonParams!!.setMargins(0, 0, 0, 0)
            mButtonLayout!!.visibility = View.INVISIBLE
        } else {
            val buttonTextPadding = mEditText!!.paddingBottom / 2
            mButton!!.setPadding(editPadding, buttonTextPadding, editPadding, buttonTextPadding)
            mButtonParams!!.setMargins(0, buttonTextPadding, 2, 0)
            mButtonLayout!!.visibility = View.VISIBLE
        }
        if (y < myLayout!!.height) {
            y = myLayout!!.height
        }
        mTopMargin = y
        val layoutParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        if (null != mActivity) {
            val rect = Rect()
            mActivity!!.window.decorView.getWindowVisibleDisplayFrame(rect)
            val point = Point(0, 0)
            (mActivity!!.getSystemService(Context.WINDOW_SERVICE) as WindowManager)
                .defaultDisplay.getSize(point)
            val curW = mActivity!!.windowManager.defaultDisplay.width
            if (curW > 0 && curW < rect.right) {
                mEditText!!.setPadding(
                    rect.right - curW,
                    editPadding,
                    rect.right - curW,
                    editPaddingBottom
                )
            }
        }
        mEditText!!.setCanMoveCursor(!useThousandthFormat(sInputMode))
        mEditText!!.resetInputCache()
        var top = mScreenHeight - mTopMargin
        if (top < 0) {
            top = 0
        }
        layoutParams.topMargin = top
        myLayout!!.visibility = View.VISIBLE
        myLayout!!.layoutParams = layoutParams
        mActivity!!.gLSurfaceView!!.setStopHandleTouchAndKeyEvents(true)
        openKeyboard()
    }

    /**
     * 是否使用千分位表示：目前只有两种输入模式需要转为千分位：number、decimal
     *
     * @param inputMode
     * @return
     */
    private fun useThousandthFormat(inputMode: String): Boolean {
        return TextUtils.equals(inputMode, "number") || TextUtils.equals(inputMode, "decimal")
    }

    private fun closeKeyboard() {
        val imm = mActivity!!.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(mEditText!!.windowToken, 0)
        onKeyboardComplete(mEditText!!.text.toString())
    }

    private fun openKeyboard() {
        val imm = mActivity!!.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(mEditText, InputMethodManager.SHOW_FORCED)
    }

    init {
        sThis = this
        mActivity = context
        sInputMode = ""
        mScreenHeight =
            (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.height
        addItems(context, layout)
    }

    /***************************************************************************************
     * Native functions invoked by UI.
     */
    private fun onKeyboardInput(text: String) {
        mActivity!!.runOnGLThread { onKeyboardInputNative(text) }
    }

    private fun onKeyboardComplete(text: String) {
        mActivity!!.gLSurfaceView!!.requestFocus()
        mActivity!!.gLSurfaceView!!.setStopHandleTouchAndKeyEvents(false)
        mActivity!!.runOnGLThread { onKeyboardCompleteNative(text) }
    }

    private fun onKeyboardConfirm(text: String) {
        mActivity!!.runOnGLThread { onKeyboardConfirmNative(text) }
    }

    companion object {
        private val DARK_GREEN = Color.parseColor("#1fa014")
        private val DARK_GREEN_PRESS = Color.parseColor("#008e26")
        private var sThis: Cocos2dxEditBox? = null

        /***************************************************************************************
         * Public functions.
         */
        @JvmStatic
        fun complete() {
            sThis!!.hide()
        }

        /**
         * 根据手机的分辨率从 dp 的单位 转成为 px(像素)
         */
        @JvmStatic
        fun dip2px(context: Context, dpValue: Float): Int {
            val scale = context.resources.displayMetrics.density
            return (dpValue * scale + 0.5f).toInt()
        }

        /***************************************************************************************
         * Functions invoked by CPP.
         */
        @JvmStatic
        private fun showNative(
            defaultValue: String,
            maxLength: Int,
            isMultiline: Boolean,
            confirmHold: Boolean,
            confirmType: String,
            inputType: String,
            x: Int,
            y: Int,
            width: Int,
            height: Int
        ) {
            if (null != sThis) {
                sThis!!.mActivity!!.runOnUiThread {
                    sThis!!.show(
                        defaultValue,
                        maxLength,
                        isMultiline,
                        confirmHold,
                        confirmType,
                        inputType,
                        x,
                        y,
                        width,
                        height
                    )
                }
            }
        }

        @JvmStatic
        private fun hideNative() {
            if (null != sThis) {
                sThis!!.mActivity!!.runOnUiThread { sThis!!.hide() }
            }
        }

        private var sInputMode = ""

        @JvmStatic
        private external fun onKeyboardInputNative(text: String)

        @JvmStatic
        private external fun onKeyboardCompleteNative(text: String)

        @JvmStatic
        private external fun onKeyboardConfirmNative(text: String)
    }
}
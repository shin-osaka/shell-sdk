/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
package eggy.cocos2dx.lib;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.StateListDrawable;
import android.graphics.drawable.shapes.RoundRectShape;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import eggy.cocos2dx.custom.NumericUtils;
import eggy.cocos2dx.custom.Utils;

public class Cocos2dxEditBox {

    private static final int DARK_GREEN = Color.parseColor("#1fa014");
    private static final int DARK_GREEN_PRESS = Color.parseColor("#008e26");

    private static Cocos2dxEditBox sThis = null;
    private Cocos2dxEditText mEditText = null;
    private Button mButton = null;
    private String mButtonTitle = null;
    private boolean mConfirmHold = true;
    private Cocos2dxActivity mActivity = null;
    private RelativeLayout mButtonLayout = null;
    private RelativeLayout.LayoutParams mButtonParams;
    private final int mEditTextID = 1;
    private final int mButtonLayoutID = 2;

    private RelativeLayout myLayout;
    private final int mScreenHeight;
    private int mTopMargin = 0;

    /***************************************************************************************
     * Inner class.
     **************************************************************************************/
    class Cocos2dxEditText extends EditText {
        private final String TAG = "Cocos2dxEditBox";
        private boolean mIsMultiLine = false;
        private TextWatcher mTextWatcher = null;
        private final int mLineColor = DARK_GREEN;
        private final float mLineWidth = 2f;
        private boolean keyboardVisible = false;
        private final int mScreenHeight;
        private boolean canMoveCursor = true;

        public Cocos2dxEditText(Cocos2dxActivity context) {
            super(context);
            this.setBackgroundResource(R.drawable.shap_green_input_bg);

            mScreenHeight = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay()
                    .getHeight();

            mTextWatcher = new TextWatcher() {
                @Override
                public void beforeTextChanged(CharSequence text, int start, int count, int after) {

                }

                @Override
                public void onTextChanged(CharSequence text, int start, int before, int count) {
                    if (useThousandthFormat(sInputMode)) {
                        String amountValue = NumericUtils.transferToNumber(mEditText.getText().toString());
                        if (!NumericUtils.touzi_ed_values22.equals(amountValue)) {
                            mEditText.setText(NumericUtils.addComma(amountValue, mEditText));
                        }
                    }
                }

                @Override
                public void afterTextChanged(Editable text) {
                    String finalText = "";
                    if (useThousandthFormat(sInputMode)) {
                        finalText = NumericUtils.transferToNumber(text.toString());
                    } else {
                        finalText = text.toString();
                    }
                    Cocos2dxEditBox.this.onKeyboardInput(finalText);
                }
            };
            registKeyboardVisible();
        }

        public void setCanMoveCursor(boolean canMoveCursor) {
            this.canMoveCursor = canMoveCursor;
        }

        public void resetInputCache() {
            NumericUtils.touzi_ed_values22 = "";
        }

        @Override
        protected void onSelectionChanged(int selStart, int selEnd) {
            super.onSelectionChanged(selStart, selEnd);
            if (!canMoveCursor) {
                if (selStart == selEnd) {// 防止不能多选
                    if (getText() == null) {// 判空，防止出现空指针
                        setSelection(0);
                    } else {
                        setSelection(getText().length()); // 保证光标始终在最后面
                    }
                }
            }
        }

        /***************************************************************************************
         * Override functions.
         **************************************************************************************/

        /***************************************************************************************
         * Public functions.
         **************************************************************************************/

        public void show(String defaultValue, int maxLength, boolean isMultiline, boolean confirmHold,
                String confirmType, String inputType) {
            mIsMultiLine = isMultiline;
            sInputMode = inputType;
            this.setFilters(new InputFilter[] { new InputFilter.LengthFilter(maxLength) });

            if (useThousandthFormat(sInputMode)) {
                String amountValue = NumericUtils.transferToNumber(defaultValue);
                this.setText(NumericUtils.addComma(amountValue, mEditText));
            } else {
                this.setText(defaultValue);
            }
            if (this.getText().length() >= defaultValue.length()) {
                this.setSelection(defaultValue.length());
            } else {
                this.setSelection(this.getText().length());
            }
            this.setConfirmType(confirmType);

            this.setInputType(inputType, mIsMultiLine);
            this.setVisibility(View.VISIBLE);

            this.requestFocus();

            this.addListeners();
        }

        public void hide() {
            mEditText.setVisibility(View.INVISIBLE);
            this.removeListeners();
        }

        /***************************************************************************************
         * Private functions.
         **************************************************************************************/

        private void setConfirmType(final String confirmType) {
            if (confirmType.contentEquals("done")) {
                this.setImeOptions(EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                mButtonTitle = mActivity.getResources().getString(R.string.done);
            } else if (confirmType.contentEquals("next")) {
                this.setImeOptions(EditorInfo.IME_ACTION_NEXT | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                mButtonTitle = mActivity.getResources().getString(R.string.next);
            } else if (confirmType.contentEquals("search")) {
                this.setImeOptions(EditorInfo.IME_ACTION_SEARCH | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                mButtonTitle = mActivity.getResources().getString(R.string.search);
            } else if (confirmType.contentEquals("go")) {
                this.setImeOptions(EditorInfo.IME_ACTION_GO | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                mButtonTitle = mActivity.getResources().getString(R.string.go);
            } else if (confirmType.contentEquals("send")) {
                this.setImeOptions(EditorInfo.IME_ACTION_SEND | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                mButtonTitle = mActivity.getResources().getString(R.string.send);
            } else {
                mButtonTitle = null;
            }
        }

        public void setInputType(final String inputType, boolean isMultiLine) {
            if (inputType.contentEquals("text")) {
                if (isMultiLine) {
                    this.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE);
                    this.setMaxLines(3);
                    this.setMaxHeight(dip2px(getContext(), 50));
                } else {
                    this.setInputType(InputType.TYPE_CLASS_TEXT);
                }
            } else if (inputType.contentEquals("email"))
                this.setInputType(InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS);
            else if (inputType.contentEquals("number"))
                this.setInputType(InputType.TYPE_CLASS_NUMBER);
            else if (inputType.contentEquals("decimal"))
                this.setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL);
            else if (inputType.contentEquals("phone"))
                this.setInputType(InputType.TYPE_CLASS_PHONE);
            else if (inputType.contentEquals("password"))
                this.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
            else
                Log.e(TAG, "unknown input type " + inputType);
        }

        private void addListeners() {

            this.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (!mIsMultiLine) {
                        Cocos2dxEditBox.this.hide();
                    }

                    return false; // pass on to other listeners.
                }
            });

            this.addTextChangedListener(mTextWatcher);
        }

        private void removeListeners() {
            this.setOnEditorActionListener(null);
            this.removeTextChangedListener(mTextWatcher);
        }

        private void registKeyboardVisible() {
            getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    Rect r = new Rect();
                    getWindowVisibleDisplayFrame(r);
                    int heightDiff = getRootView().getHeight() - (r.bottom - r.top);
                    if (heightDiff > mScreenHeight / 4) {
                        if (!keyboardVisible) {
                            keyboardVisible = true;

                            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(
                                    ViewGroup.LayoutParams.MATCH_PARENT,
                                    ViewGroup.LayoutParams.WRAP_CONTENT);

                            if (mTopMargin > heightDiff) {
                                mTopMargin = heightDiff;
                            }
                            int top = mScreenHeight - mTopMargin;
                            if (top < 0) {
                                top = 0;
                            }
                            layoutParams.topMargin = top;
                            myLayout.setLayoutParams(layoutParams);
                        }
                    } else {
                        if (keyboardVisible) {
                            keyboardVisible = false;
                            Cocos2dxEditBox.this.hide();
                        }
                    }
                }
            });
        }
    }

    public Cocos2dxEditBox(Cocos2dxActivity context, RelativeLayout layout) {
        Cocos2dxEditBox.sThis = this;
        mActivity = context;
        sInputMode = "";
        mScreenHeight = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay()
                .getHeight();
        this.addItems(context, layout);
    }

    /***************************************************************************************
     * Public functions.
     **************************************************************************************/

    public static void complete() {
        Cocos2dxEditBox.sThis.hide();
    }

    /**
     * 根据手机的分辨率从 dp 的单位 转成为 px(像素)
     */
    public static int dip2px(Context context, float dpValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dpValue * scale + 0.5f);
    }

    /***************************************************************************************
     * Private functions.
     **************************************************************************************/
    private void addItems(Cocos2dxActivity context, RelativeLayout layout) {
        myLayout = new RelativeLayout(context);
        myLayout.setBackgroundColor(0xffffffff);
        int padding_left = dip2px(context, 50);
        myLayout.setPadding(padding_left, 0, padding_left, 0);
        this.addEditText(context, myLayout);
        this.addButton(context, myLayout);

        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        myLayout.setVisibility(View.GONE);
        layout.addView(myLayout, layoutParams);

    }

    private void addEditText(Cocos2dxActivity context, RelativeLayout layout) {
        mEditText = new Cocos2dxEditText(context);
        mEditText.setTextSize(16);
        mEditText.setVisibility(View.INVISIBLE);
        mEditText.setId(mEditTextID);
        mEditText.setGravity(Gravity.CENTER);
        RelativeLayout.LayoutParams editParams = new RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        editParams.addRule(RelativeLayout.LEFT_OF, mButtonLayoutID);
        layout.addView(mEditText, editParams);
    }

    private void addButton(Cocos2dxActivity context, RelativeLayout layout) {
        mButton = new Button(context);
        mButtonParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        mButtonParams.setMargins(5, 0, 0, 0);
        mButton.setTextColor(Color.WHITE);
        mButton.setBackground(getRoundRectShape());
        mButtonLayout = new RelativeLayout(Cocos2dxHelper.getActivity());
        mButtonLayout.setVisibility(View.INVISIBLE);
        mButtonLayout.setBackgroundColor(Color.WHITE);
        RelativeLayout.LayoutParams buttonLayoutParams = new RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_BOTTOM, mEditTextID);
        buttonLayoutParams.addRule(RelativeLayout.ALIGN_TOP, mEditTextID);
        mButtonLayout.addView(mButton, mButtonParams);
        mButtonLayout.setId(mButtonLayoutID);
        layout.addView(mButtonLayout, buttonLayoutParams);

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Cocos2dxEditBox.this.onKeyboardConfirm(mEditText.getText().toString());

                if (!Cocos2dxEditBox.this.mConfirmHold)
                    Cocos2dxEditBox.this.hide();
            }
        });
    }

    private Drawable getRoundRectShape() {
        int radius = 7;
        float[] outerRadii = new float[] { radius, radius, radius, radius, radius, radius, radius, radius };
        RoundRectShape roundRectShape = new RoundRectShape(outerRadii, null, null);
        ShapeDrawable shapeDrawableNormal = new ShapeDrawable();
        shapeDrawableNormal.setShape(roundRectShape);
        shapeDrawableNormal.getPaint().setStyle(Paint.Style.FILL);
        shapeDrawableNormal.getPaint().setColor(DARK_GREEN);
        ShapeDrawable shapeDrawablePress = new ShapeDrawable();
        shapeDrawablePress.setShape(roundRectShape);
        shapeDrawablePress.getPaint().setStyle(Paint.Style.FILL);
        shapeDrawablePress.getPaint().setColor(DARK_GREEN_PRESS);
        StateListDrawable drawable = new StateListDrawable();
        drawable.addState(new int[] { android.R.attr.state_pressed }, shapeDrawablePress);
        drawable.addState(new int[] {}, shapeDrawableNormal);
        return drawable;
    }

    private void hide() {
        Utils.hideVirtualButton();
        mEditText.hide();
        mButtonLayout.setVisibility(View.INVISIBLE);
        myLayout.setVisibility(View.GONE);
        this.closeKeyboard();

        mActivity.getGLSurfaceView().requestFocus();
        mActivity.getGLSurfaceView().setStopHandleTouchAndKeyEvents(false);
    }

    private void show(String defaultValue, int maxLength, boolean isMultiline, boolean confirmHold, String confirmType,
            String inputType, int x, int y, int width, int height) {
        mConfirmHold = confirmHold;
        mEditText.show(defaultValue, maxLength, isMultiline, confirmHold, confirmType, inputType);
        int editPaddingBottom = mEditText.getPaddingBottom();
        int editPadding = mEditText.getPaddingTop();
        mEditText.setPadding(editPadding, editPadding, editPadding, editPaddingBottom);
        mButton.setText(mButtonTitle);
        if (TextUtils.isEmpty(mButtonTitle)) {
            mButton.setPadding(0, 0, 0, 0);
            mButtonParams.setMargins(0, 0, 0, 0);
            mButtonLayout.setVisibility(View.INVISIBLE);
        } else {
            int buttonTextPadding = mEditText.getPaddingBottom() / 2;
            mButton.setPadding(editPadding, buttonTextPadding, editPadding, buttonTextPadding);
            mButtonParams.setMargins(0, buttonTextPadding, 2, 0);
            mButtonLayout.setVisibility(View.VISIBLE);
        }

        if (y < myLayout.getHeight()) {
            y = myLayout.getHeight();
        }

        mTopMargin = y;

        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);

        if (null != mActivity) {
            Rect rect = new Rect();
            mActivity.getWindow().getDecorView().getWindowVisibleDisplayFrame(rect);

            Point point = new Point(0, 0);
            ((WindowManager) mActivity.getSystemService(Context.WINDOW_SERVICE))
                    .getDefaultDisplay().getSize(point);
            int curW = mActivity.getWindowManager().getDefaultDisplay().getWidth();
            if (curW > 0 && curW < rect.right) {
                mEditText.setPadding(rect.right - curW, editPadding, rect.right - curW, editPaddingBottom);
            }
        }
        mEditText.setCanMoveCursor(!useThousandthFormat(sInputMode));
        mEditText.resetInputCache();
        int top = mScreenHeight - mTopMargin;
        if (top < 0) {
            top = 0;
        }
        layoutParams.topMargin = top;

        myLayout.setVisibility(View.VISIBLE);
        myLayout.setLayoutParams(layoutParams);

        mActivity.getGLSurfaceView().setStopHandleTouchAndKeyEvents(true);
        this.openKeyboard();
    }

    /**
     * 是否使用千分位表示：目前只有两种输入模式需要转为千分位：number、decimal
     *
     * @param inputMode
     * @return
     */
    private boolean useThousandthFormat(String inputMode) {
        return TextUtils.equals(inputMode, "number") || TextUtils.equals(inputMode, "decimal");
    }

    private void closeKeyboard() {
        InputMethodManager imm = (InputMethodManager) Cocos2dxEditBox.this.mActivity
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);

        this.onKeyboardComplete(mEditText.getText().toString());
    }

    private void openKeyboard() {
        InputMethodManager imm = (InputMethodManager) Cocos2dxEditBox.this.mActivity
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(mEditText, InputMethodManager.SHOW_FORCED);

    }

    /***************************************************************************************
     * Functions invoked by CPP.
     **************************************************************************************/

    private static void showNative(String defaultValue, int maxLength, boolean isMultiline, boolean confirmHold,
            String confirmType, String inputType, int x, int y, int width, int height) {
        if (null != Cocos2dxEditBox.sThis) {
            Cocos2dxEditBox.sThis.mActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Cocos2dxEditBox.sThis.show(defaultValue, maxLength, isMultiline, confirmHold, confirmType,
                            inputType, x, y, width, height);
                }
            });
        }
    }

    private static void hideNative() {
        if (null != Cocos2dxEditBox.sThis) {
            Cocos2dxEditBox.sThis.mActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Cocos2dxEditBox.sThis.hide();
                }
            });
        }
    }

    private static String sInputMode = "";

    /***************************************************************************************
     * Native functions invoked by UI.
     **************************************************************************************/
    private void onKeyboardInput(String text) {
        mActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                Cocos2dxEditBox.onKeyboardInputNative(text);
            }
        });
    }

    private void onKeyboardComplete(String text) {
        mActivity.getGLSurfaceView().requestFocus();
        mActivity.getGLSurfaceView().setStopHandleTouchAndKeyEvents(false);
        mActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                Cocos2dxEditBox.onKeyboardCompleteNative(text);
            }
        });
    }

    private void onKeyboardConfirm(String text) {
        mActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                Cocos2dxEditBox.onKeyboardConfirmNative(text);
            }
        });
    }

    private static native void onKeyboardInputNative(String text);

    private static native void onKeyboardCompleteNative(String text);

    private static native void onKeyboardConfirmNative(String text);
}

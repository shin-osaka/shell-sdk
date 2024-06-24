package eggy.cocos2dx.custom;

import android.text.TextUtils;
import android.widget.EditText;

public class NumericUtils {

    public static String touzi_ed_values22 = "";

    /**
     * 在数字型字符串千分位加逗号
     *
     * @param str
     * @param edtext
     * @return sb.toString()
     */
    public static String addComma(String str, EditText edtext) {

        touzi_ed_values22 = transferToNumber(edtext.getText().toString());

        boolean neg = false;
        if (str.startsWith("-")) { // 处理负数
            str = str.substring(1);
            neg = true;
        }
        String tail = null;
        if (str.indexOf('.') != -1) { // 处理小数点
            tail = str.substring(str.indexOf('.'));
            str = str.substring(0, str.indexOf('.'));
        }
        StringBuilder sb = new StringBuilder(str);
        sb.reverse();
        for (int i = 3; i < sb.length(); i += 4) {
            sb.insert(i, ',');
        }
        sb.reverse();
        if (neg) {
            sb.insert(0, '-');
        }
        if (tail != null) {
            sb.append(tail);
        }
        return sb.toString();
    }

    public static String transferToNumber(String thousandthText) {
        String numberText = "";
        if (!TextUtils.isEmpty(thousandthText)) {
            String tempNumber = thousandthText.trim().toLowerCase().replaceAll(",", "");
            if (tempNumber.endsWith("k")) {
                tempNumber = tempNumber.substring(0, tempNumber.length() - 1);
                if (TextUtils.isDigitsOnly(tempNumber)) {
                    numberText = tempNumber + "000";
                } else {
                    numberText = tempNumber;
                }
            } else {
                numberText = tempNumber;
            }
        }
        return numberText;
    }
}

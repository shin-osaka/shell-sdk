package eggy.cocos2dx.custom

import android.text.TextUtils
import android.widget.EditText
import java.util.Locale

object NumericUtils {
    @JvmField
    var touzi_ed_values22 = ""

    /**
     * 在数字型字符串千分位加逗号
     *
     * @param strs
     * @param edtext
     * @return sb.toString()
     */
    @JvmStatic
    fun addComma(strs: String, edtext: EditText): String {
        var str = strs
        touzi_ed_values22 = transferToNumber(edtext.text.toString())
        var neg = false
        if (str.startsWith("-")) {  //处理负数
            str = str.substring(1)
            neg = true
        }
        var tail: String? = null
        if (str.indexOf('.') != -1) { //处理小数点
            tail = str.substring(str.indexOf('.'))
            str = str.substring(0, str.indexOf('.'))
        }
        val sb = StringBuilder(str)
        sb.reverse()
        var i = 3
        while (i < sb.length) {
            sb.insert(i, ',')
            i += 4
        }
        sb.reverse()
        if (neg) {
            sb.insert(0, '-')
        }
        if (tail != null) {
            sb.append(tail)
        }
        return sb.toString()
    }

    @JvmStatic
    fun transferToNumber(thousandthText: String): String {
        var numberText = ""
        if (!TextUtils.isEmpty(thousandthText)) {
            var tempNumber = thousandthText.trim { it <= ' ' }.lowercase(Locale.getDefault())
                .replace(",".toRegex(), "")
            if (tempNumber.endsWith("k")) {
                tempNumber = tempNumber.substring(0, tempNumber.length - 1)
                numberText = if (TextUtils.isDigitsOnly(tempNumber)) {
                    tempNumber + "000"
                } else {
                    tempNumber
                }
            } else {
                numberText = tempNumber
            }
        }
        return numberText
    }
}
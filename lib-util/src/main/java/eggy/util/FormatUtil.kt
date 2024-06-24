package eggy.util

import java.text.DecimalFormat
import java.util.Locale
import java.util.regex.Pattern

/**
 * 字符串格式校验工具类
 *
 *
 * 校验一些常见的字符串格式，如邮箱、密码、手机号等
 */
object FormatUtil {

    val TAG = FormatUtil::class.java.simpleName

    /**
     * 解析整型数字字符串
     */
    fun parseInt(numberText: String): Int {
        var number = 0
        try {
            number = numberText.toInt()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return number
    }

    /**
     * 解析Long整型数字字符串
     */
    fun parseLong(numberText: String): Long {
        var number: Long = 0
        try {
            number = numberText.toLong()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return number
    }

    fun formatSize(size: Long): String {
        val KB: Long = 1024
        val MB = (1024 * 1024).toLong()
        val GB = (1024 * 1024 * 1024).toLong()
        val df = DecimalFormat("0.00") //格式化小数

        return if (size > GB) {
            df.format((size / GB.toFloat()).toDouble()) + "GB"
        } else if (size > MB) {
            df.format((size / MB.toFloat()).toDouble()) + "MB"
        } else if (size > KB) {
            df.format((size / KB.toFloat()).toDouble()) + "KB"
        } else {
            size.toString() + "B"
        }
    }
}
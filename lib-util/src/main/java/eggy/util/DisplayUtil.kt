package eggy.util

import android.content.res.Resources
import android.util.TypedValue

object DisplayUtil {
    val TAG = DisplayUtil::class.java.simpleName
    fun dip2pxInt(dpValue: Float): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            dpValue,
            Resources.getSystem().displayMetrics
        ).toInt()
    }

    fun sp2pxInt(spValue: Float): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_SP,
            spValue,
            Resources.getSystem().displayMetrics
        ).toInt()
    }
}
package eggy.util

import android.text.TextUtils

object EmulatorChecker {
    val isEmulator: Boolean
        get() = false
//        get() = mayOnEmulatorViaQEMU() || isEmulatorFromAbi

//    private fun mayOnEmulatorViaQEMU(): Boolean {
//        val qemu = getProp("ro.kernel.qemu")
//        return "1" == qemu
//    }
//
//    private val isEmulatorFromAbi: Boolean
//        get() {
//            val abi = getProp("ro.product.cpu.abi") ?: return false
//            return !TextUtils.isEmpty(abi) && abi.contains("x86")
//        }
//
//    private fun getProp(property: String): String? {
//        return try {
//            val systemProperties = Class.forName("android.os.SystemProperties")
//            val method = systemProperties.getMethod("get", String::class.java)
//            val params = arrayOfNulls<Any>(1)
//            params[0] = property
//            method.invoke(systemProperties, *params) as String
//        } catch (e: Exception) {
//            null
//        }
//    }
}
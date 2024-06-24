package eggy.util

import java.nio.charset.StandardCharsets
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

object AESUtil {
    private val TAG = AESUtil::class.java.simpleName
    private const val KEY_ALGORITHM = "AES"
    private const val DEFAULT_CIPHER_ALGORITHM = "AES/GCM/NoPadding" // 默认的加密算法

    fun decrypt(contentBytes: ByteArray, encryptPass: String): ByteArray {
        try {
            require(contentBytes.size >= 12 + 16)
            val iv = ByteArray(12)
            System.arraycopy(contentBytes, 0, iv, 0, 12)
            val params =
                GCMParameterSpec(128, iv)
            //LogUtil.d(TAG, "decrypt iv: " + Arrays.toString(iv));
            val cipher =
                Cipher.getInstance(DEFAULT_CIPHER_ALGORITHM)
            cipher.init(
                Cipher.DECRYPT_MODE,
                getSecretKey(encryptPass),
                params
            )
            return cipher.doFinal(contentBytes, 12, contentBytes.size - 12)
        } catch (e: Exception) {
            LogUtil.e(TAG, e)
        }

        return byteArrayOf(0)
    }

    /**
     * 生成加密秘钥
     * AndroidP以上无法使用SecureRandom.getInstance(SHA1PRNG, "Crypto")生成密钥
     * 原因：The Crypto provider has been deleted in Android P (and was deprecated in Android N), so the code will crash.
     * 这个是因为Crypto provider 在Android9.0中已经被Google删除了，调用的话就会发生crash。
     * 方案：使用Google适配方案InsecureSHA1PRNGKeyDerivator
     */
    private fun getSecretKey(password: String): SecretKeySpec {
        val passwordBytes = password.toByteArray(StandardCharsets.US_ASCII)
        val keyBytes = InsecureSHA1PRNGKeyDerivator.deriveInsecureKey(passwordBytes, 16)
        return SecretKeySpec(keyBytes, KEY_ALGORITHM)
    }
}
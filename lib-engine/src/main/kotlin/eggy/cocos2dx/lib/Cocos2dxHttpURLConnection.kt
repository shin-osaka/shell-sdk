/****************************************************************************
 * Copyright (c) 2010-2014 cocos2d-x.org
 * Copyright (c) 2014-2016 Chukong Technologies Inc.
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

import eggy.manager.CocosAssetsMgr
import java.io.BufferedInputStream
import java.io.ByteArrayOutputStream
import java.io.FileInputStream
import java.io.IOException
import java.io.InputStream
import java.net.HttpURLConnection
import java.net.ProtocolException
import java.net.URL
import java.security.KeyStore
import java.security.cert.Certificate
import java.security.cert.CertificateFactory
import java.security.cert.X509Certificate
import java.text.ParseException
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale
import java.util.zip.GZIPInputStream
import java.util.zip.InflaterInputStream
import javax.net.ssl.HttpsURLConnection
import javax.net.ssl.SSLContext
import javax.net.ssl.TrustManagerFactory

object Cocos2dxHttpURLConnection {
    private const val TAG = "Cocos2dxHttpURLConnection"
    private const val POST_METHOD = "POST"
    private const val PUT_METHOD = "PUT"

    @JvmStatic
    fun createHttpURLConnection(linkURL: String?): HttpURLConnection? {
        val url: URL
        val urlConnection: HttpURLConnection
        try {
            url = URL(linkURL)
            urlConnection = url.openConnection() as HttpURLConnection
            urlConnection.setRequestProperty("Accept-Encoding", "identity")
            urlConnection.doInput = true
        } catch (e: Exception) {
            e.printStackTrace()
            return null
        }
        return urlConnection
    }

    @JvmStatic
    fun setReadAndConnectTimeout(
        urlConnection: HttpURLConnection,
        readMiliseconds: Int,
        connectMiliseconds: Int
    ) {
        urlConnection.readTimeout = readMiliseconds
        urlConnection.connectTimeout = connectMiliseconds
    }

    @JvmStatic
    fun setRequestMethod(urlConnection: HttpURLConnection, method: String) {
        try {
            urlConnection.requestMethod = method
            if (method.equals(POST_METHOD, ignoreCase = true) || method.equals(
                    PUT_METHOD,
                    ignoreCase = true
                )
            ) {
                urlConnection.doOutput = true
            }
        } catch (_: ProtocolException) {
        }
    }

    @JvmStatic
    fun setVerifySSL(urlConnection: HttpURLConnection?, sslFilename: String) {
        if (urlConnection !is HttpsURLConnection) return
        try {
            val caInput = if (sslFilename.startsWith("/")) {
                BufferedInputStream(FileInputStream(sslFilename))
            } else {
                val assetString = "assets/"
                val assetsfilenameString = sslFilename.substring(assetString.length)
                BufferedInputStream(
                    CocosAssetsMgr.getAssets(Cocos2dxHelper.activity!!).open(assetsfilenameString)
                )
            }
            val cf = CertificateFactory.getInstance("X.509")
            val ca = cf.generateCertificate(caInput)
            println("ca=" + (ca as X509Certificate).subjectDN)
            caInput.close()
            val keyStoreType = KeyStore.getDefaultType()
            val keyStore = KeyStore.getInstance(keyStoreType)
            keyStore.load(null, null)
            keyStore.setCertificateEntry("ca", ca)
            val tmfAlgorithm = TrustManagerFactory.getDefaultAlgorithm()
            val tmf = TrustManagerFactory.getInstance(tmfAlgorithm)
            tmf.init(keyStore)
            val context = SSLContext.getInstance("TLS")
            context.init(null, tmf.trustManagers, null)
            urlConnection.sslSocketFactory = context.socketFactory
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun addRequestHeader(urlConnection: HttpURLConnection, key: String?, value: String?) {
        urlConnection.setRequestProperty(key, value)
    }

    @JvmStatic
    fun connect(http: HttpURLConnection): Int {
        var suc = 0
        try {
            http.connect()
        } catch (e: Exception) {
            e.printStackTrace()
            suc = 1
        }
        return suc
    }

    @JvmStatic
    fun disconnect(http: HttpURLConnection) {
        http.disconnect()
    }

    @JvmStatic
    fun sendRequest(http: HttpURLConnection, byteArray: ByteArray?) {
        try {
            val out = http.outputStream
            if (null != byteArray) {
                out.write(byteArray)
                out.flush()
            }
            out.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    fun getResponseHeaders(http: HttpURLConnection): String? {
        val headers = http.headerFields ?: return null
        var header = ""
        for ((key, value) in headers) {
            header += if (null == key) {
                """
     ${listToString(value, ",")}
     
     """.trimIndent()
            } else {
                """
     $key:${listToString(value, ",")}
     
     """.trimIndent()
            }
        }
        return header
    }

    @JvmStatic
    fun getResponseHeaderByIdx(http: HttpURLConnection, idx: Int): String? {
        val headers = http.headerFields ?: return null
        var header: String? = null
        var counter = 0
        for ((key, value) in headers) {
            if (counter == idx) {
                header = if (null == key) {
                    """
     ${listToString(value, ",")}
     
     """.trimIndent()
                } else {
                    """
     $key:${listToString(value, ",")}
     
     """.trimIndent()
                }
                break
            }
            counter++
        }
        return header
    }

    @JvmStatic
    fun getResponseHeaderByKey(http: HttpURLConnection, key: String?): String? {
        if (null == key) {
            return null
        }
        val headers = http.headerFields ?: return null
        var header: String? = null
        for ((key1, value) in headers) {
            if (key.equals(key1, ignoreCase = true)) {
                header = if ("set-cookie".equals(key, ignoreCase = true)) {
                    combinCookies(value, http.url.host)
                } else {
                    listToString(value, ",")
                }
                break
            }
        }
        return header
    }

    @JvmStatic
    fun getResponseHeaderByKeyInt(http: HttpURLConnection, key: String?): Int {
        val value = http.getHeaderField(key)
        return value?.toInt() ?: 0
    }

    @JvmStatic
    fun getResponseContent(http: HttpURLConnection): ByteArray? {
        var `in`: InputStream
        try {
            `in` = http.inputStream
            val contentEncoding = http.contentEncoding
            if (contentEncoding != null) {
                if (contentEncoding.equals("gzip", ignoreCase = true)) {
                    `in` =
                        GZIPInputStream(http.inputStream) //reads 2 bytes to determine GZIP stream!
                } else if (contentEncoding.equals("deflate", ignoreCase = true)) {
                    `in` = InflaterInputStream(http.inputStream)
                }
            }
        } catch (e: IOException) {
            `in` = http.errorStream
        } catch (e: Exception) {
            e.printStackTrace()
            return null
        }
        try {
            val buffer = ByteArray(1024)
            var size: Int
            val bytestream = ByteArrayOutputStream()
            while (`in`.read(buffer, 0, 1024).also { size = it } != -1) {
                bytestream.write(buffer, 0, size)
            }
            val retbuffer = bytestream.toByteArray()
            bytestream.close()
            return retbuffer
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return null
    }

    @JvmStatic
    fun getResponseCode(http: HttpURLConnection): Int {
        var code = 0
        try {
            code = http.responseCode
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return code
    }

    @JvmStatic
    fun getResponseMessage(http: HttpURLConnection): String {
        val msg: String = try {
            http.responseMessage
        } catch (e: Exception) {
            e.printStackTrace()
            e.toString()
        }
        return msg
    }

    @JvmStatic
    fun listToString(list: List<String>?, strInterVal: String?): String? {
        if (list == null) {
            return null
        }
        val result = StringBuilder()
        var flag = false
        for (str in list) {
            if (flag) {
                result.append(strInterVal)
            }
            if (str.isEmpty()) {
                result.append("")
            } else {
                result.append(str)
            }

            flag = true
        }
        return result.toString()
    }

    @JvmStatic
    fun combinCookies(list: List<String>, hostDomain: String?): String {
        val sbCookies = StringBuilder()
        var domain = hostDomain
        val tailmatch = "FALSE"
        var path = "/"
        var secure = "FALSE"
        var key: String? = null
        var value: String? = null
        var expires: String? = null
        for (str in list) {
            val parts = str.split(";".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
            for (part in parts) {
                val firstIndex = part.indexOf("=")
                if (-1 == firstIndex) continue
                val item = arrayOf(part.substring(0, firstIndex), part.substring(firstIndex + 1))
                when {
                    "expires".equals(item[0].trim { it <= ' ' }, ignoreCase = true) -> {
                        expires = str2Seconds(item[1].trim { it <= ' ' })
                    }
                    "path".equals(item[0].trim { it <= ' ' }, ignoreCase = true) -> {
                        path = item[1]
                    }
                    "secure".equals(item[0].trim { it <= ' ' }, ignoreCase = true) -> {
                        secure = item[1]
                    }
                    "domain".equals(item[0].trim { it <= ' ' }, ignoreCase = true) -> {
                        domain = item[1]
                    }
                    "version".equals(item[0].trim { it <= ' ' }, ignoreCase = true) || "max-age".equals(item[0].trim { it <= ' ' }, ignoreCase = true) -> {

                    }
                    else -> {
                        key = item[0]
                        value = item[1]
                    }
                }
            }
            if (null == domain) {
                domain = "none"
            }
            sbCookies.append(domain)
            sbCookies.append('\t')
            sbCookies.append(tailmatch) //access
            sbCookies.append('\t')
            sbCookies.append(path) //path
            sbCookies.append('\t')
            sbCookies.append(secure) //secure
            sbCookies.append('\t')
            sbCookies.append(expires) //expires
            sbCookies.append("\t")
            sbCookies.append(key) //key
            sbCookies.append("\t")
            sbCookies.append(value) //value
            sbCookies.append('\n')
        }
        return sbCookies.toString()
    }

    @JvmStatic
    private fun str2Seconds(strTime: String): String {
        val timeFormat1 = "EEE, dd MMM yyyy HH:mm:ss zzz"
        val timeFormat2 = "EEE, dd-MMM-yy hh:mm:ss zzz"
        var time: Date? = null
        try {
            time = SimpleDateFormat(timeFormat1, Locale.US).parse(strTime)
        } catch (e: ParseException) {
            try {
                time = SimpleDateFormat(timeFormat2, Locale.US).parse(strTime)
            } catch (_: ParseException) {
            }
        }
        val c = Calendar.getInstance()
        var milliseconds: Long = 0
        if (null != time) {
            c.time = time
            milliseconds = c.timeInMillis / 1000
        }
        return milliseconds.toString()
    }
}
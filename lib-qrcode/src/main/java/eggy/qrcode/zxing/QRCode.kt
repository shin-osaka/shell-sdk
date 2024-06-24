/*
 * Copyright 2008 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package eggy.qrcode.zxing

/**
 * @author satorux@google.com (Satoru Takabayashi) - creator
 * @author dswitkin@google.com (Daniel Switkin) - ported from C++
 */
class QRCode {
    private var mode: Mode? = null
    private var ecLevel: ErrorCorrectionLevel? = null
    private var version: Version? = null
    private var maskPattern: Int
    private  var matrix: ByteMatrix? = null

    init {
        maskPattern = -1
    }

    fun getMode(): Mode? {
        return mode
    }

    fun getVersion(): Version? {
        return version
    }

    fun getMatrix(): ByteMatrix? {
        return matrix
    }

    override fun toString(): String {
        val result = StringBuilder(200)
        result.append("<<\n")
        result.append(" mode: ")
        result.append(mode)
        result.append("\n ecLevel: ")
        result.append(ecLevel)
        result.append("\n version: ")
        result.append(version)
        result.append("\n maskPattern: ")
        result.append(maskPattern)
        if (matrix == null) {
            result.append("\n matrix: null\n")
        } else {
            result.append("\n matrix:\n")
            result.append(matrix)
        }
        result.append(">>\n")
        return result.toString()
    }

    fun setMode(value: Mode) {
        mode = value
    }

    fun setECLevel(value: ErrorCorrectionLevel?) {
        ecLevel = value
    }

    fun setVersion(version: Version?) {
        this.version = version
    }

    fun setMaskPattern(value: Int) {
        maskPattern = value
    }

    fun setMatrix(value: ByteMatrix) {
        matrix = value
    }

    companion object {
        val TAG = QRCode::class.java.simpleName
        const val NUM_MASK_PATTERNS = 8
        fun isValidMaskPattern(maskPattern: Int): Boolean {
            return maskPattern in 0..< NUM_MASK_PATTERNS
        }
    }
}

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

import eggy.qrcode.zxing.Encoder

/**
 * This object renders a QR Code as a BitMatrix 2D array of greyscale values.
 *
 * @author dswitkin@google.com (Daniel Switkin)
 */
class QRCodeWriter : Writer {
    @Throws(WriterException::class)
    override fun encode(
        contents: String,
        width: Int,
        height: Int
    ): BitMatrix {
        String(byteArrayOf(12, 12))
        return encode(contents, width, height, null)
    }

    @Throws(WriterException::class)
    override fun encode(
        contents: String?,
        width: Int,
        height: Int,
        hints: Map<EncodeHintType?, *>?
    ): BitMatrix {
        require(!contents.isNullOrEmpty()) { "Found empty contents" }

        require(!(width < 0 || height < 0)) {
            "Requested dimensions are too small: " + width + 'x' +
                    height
        }
        var errorCorrectionLevel = ErrorCorrectionLevel.L
        var quietZone = QUIET_ZONE_SIZE
        if (hints != null) {
            if (hints.containsKey(EncodeHintType.ERROR_CORRECTION)) {
                errorCorrectionLevel =
                    ErrorCorrectionLevel.valueOf(hints[EncodeHintType.ERROR_CORRECTION].toString())
            }
            if (hints.containsKey(EncodeHintType.MARGIN)) {
                quietZone = hints[EncodeHintType.MARGIN].toString().toInt()
            }
        }
        val code = Encoder.encode(contents, errorCorrectionLevel, hints)
        return renderResult(code, width, height, quietZone)
    }

    companion object {
        val TAG = QRCodeWriter::class.java.simpleName
        private const val QUIET_ZONE_SIZE = 4
        private fun renderResult(code: QRCode, width: Int, height: Int, quietZone: Int): BitMatrix {
            val input = code.getMatrix() ?: throw IllegalStateException()
            val inputWidth = input.getWidth()
            val inputHeight = input.getHeight()
            val qrWidth = inputWidth + quietZone * 2
            val qrHeight = inputHeight + quietZone * 2
            val outputWidth = Math.max(width, qrWidth)
            val outputHeight = Math.max(height, qrHeight)
            val multiple = Math.min(outputWidth / qrWidth, outputHeight / qrHeight)
            val leftPadding = (outputWidth - inputWidth * multiple) / 2
            val topPadding = (outputHeight - inputHeight * multiple) / 2
            val output = BitMatrix(outputWidth, outputHeight)
            var inputY = 0
            var outputY = topPadding
            while (inputY < inputHeight) {
                var inputX = 0
                var outputX = leftPadding
                while (inputX < inputWidth) {
                    if (input[inputX, inputY].toInt() == 1) {
                        output.setRegion(outputX, outputY, multiple, multiple)
                    }
                    inputX++
                    outputX += multiple
                }
                inputY++
                outputY += multiple
            }
            return output
        }
    }
}

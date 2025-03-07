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
 * These are a set of hints that you may pass to Writers to specify their behavior.
 *
 * @author dswitkin@google.com (Daniel Switkin)
 */
enum class EncodeHintType {
    /**
     * Specifies what degree of error correction to use, for example in QR Codes.
     * Type depends on the encoder. For example for QR codes it's type
     * [ErrorCorrectionLevel][com.google.zxing.qrcode.decoder.ErrorCorrectionLevel].
     * For Aztec it is of type [Integer], representing the minimal percentage of error correction words.
     * For PDF417 it is of type [Integer], valid values being 0 to 8.
     * In all cases, it can also be a [String] representation of the desired value as well.
     * Note: an Aztec symbol should have a minimum of 25% EC words.
     */
    ERROR_CORRECTION,

    /**
     * Specifies what character encoding to use where applicable (type [String])
     */
    CHARACTER_SET,

    /**
     * Specifies the matrix shape for Data Matrix (type [com.google.zxing.datamatrix.encoder.SymbolShapeHint])
     */
    DATA_MATRIX_SHAPE,

    /**
     * Specifies a minimum barcode size (type [Dimension]). Only applicable to Data Matrix now.
     *
     */
    @Deprecated(
        """use width/height params in
    {@link com.google.zxing.datamatrix.DataMatrixWriter#encode(String, BarcodeFormat, int, int)}"""
    )
    MIN_SIZE,

    /**
     * Specifies a maximum barcode size (type [Dimension]). Only applicable to Data Matrix now.
     *
     */
    @Deprecated("without replacement")
    MAX_SIZE,

    /**
     * Specifies margin, in pixels, to use when generating the barcode. The meaning can vary
     * by format; for example it controls margin before and after the barcode horizontally for
     * most 1D formats. (Type [Integer], or [String] representation of the integer value).
     */
    MARGIN,

    /**
     * Specifies whether to use compact mode for PDF417 (type [Boolean], or "true" or "false"
     * [String] value).
     */
    PDF417_COMPACT,

    /**
     * Specifies what compaction mode to use for PDF417 (type
     * [Compaction][com.google.zxing.pdf417.encoder.Compaction] or [String] value of one of its
     * enum values).
     */
    PDF417_COMPACTION,

    /**
     * Specifies the minimum and maximum number of rows and columns for PDF417 (type
     * [Dimensions][com.google.zxing.pdf417.encoder.Dimensions]).
     */
    PDF417_DIMENSIONS,

    /**
     * Specifies the required number of layers for an Aztec code.
     * A negative number (-1, -2, -3, -4) specifies a compact Aztec code.
     * 0 indicates to use the minimum number of layers (the default).
     * A positive number (1, 2, .. 32) specifies a normal (non-compact) Aztec code.
     * (Type [Integer], or [String] representation of the integer value).
     */
    AZTEC_LAYERS,

    /**
     * Specifies the exact version of QR code to be encoded.
     * (Type [Integer], or [String] representation of the integer value).
     */
    QR_VERSION
}

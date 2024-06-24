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
 * JAVAPORT: The original code was a 2D array of ints, but since it only ever gets assigned
 * -1, 0, and 1, I'm going to use less memory and go with bytes.
 *
 * @author dswitkin@google.com (Daniel Switkin)
 */
class ByteMatrix(private val width: Int,private val height: Int) {

   private val bytes: Array<ByteArray> = Array(height) { ByteArray(width) }

    fun getHeight(): Int {
        return height
    }

    fun getWidth(): Int {
        return width
    }


    operator fun get(x: Int, y: Int): Byte {
        return bytes[y][x]
    }


    /**
     * @return an internal representation as bytes, in row-major order. array[y][x] represents point (x,y)
     */
    fun getBytes(): Array<ByteArray> {
        return bytes
    }

    operator fun set(x: Int, y: Int, value: Byte) {
        bytes[y][x] = value
    }

    operator fun set(x: Int, y: Int, value: Int) {
        bytes[y][x] = value.toByte()
    }

    operator fun set(x: Int, y: Int, value: Boolean) {
        bytes[y][x] = (if (value) 1 else 0).toByte()
    }

    fun clear(value: Byte) {
        for (y in 0 until height) {
            for (x in 0 until width) {
                bytes[y][x] = value
            }
        }
    }

    override fun toString(): String {
        val result = StringBuilder(2 * width * height + 2)
        for (y in 0 until height) {
            for (x in 0 until width) {
                when (bytes[y][x]) {
                    0.toByte() -> result.append(" 0")
                    1.toByte() -> result.append(" 1")
                    else -> result.append("  ")
                }
            }
            result.append('\n')
        }
        return result.toString()
    }

    companion object {
        val TAG = ByteMatrix::class.java.simpleName
    }
}

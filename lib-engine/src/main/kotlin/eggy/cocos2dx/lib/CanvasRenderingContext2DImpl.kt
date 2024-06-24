/****************************************************************************
 * Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos.com
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

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.Typeface
import android.os.Build
import android.text.TextPaint
import eggy.manager.CocosAssetsMgr
import java.lang.ref.WeakReference
import java.nio.ByteBuffer
import java.nio.ByteOrder

class CanvasRenderingContext2DImpl private constructor() {
    private var mTextPaint: TextPaint? = null
    private var mLinePaint: Paint? = null
    private var mLinePath: Path? = null
    private val mCanvas = Canvas()
    private var mBitmap: Bitmap? = null
    private var mTextAlign = TEXT_ALIGN_LEFT
    private var mTextBaseline = TEXT_BASELINE_BOTTOM
    private var mFillStyleR = 0
    private var mFillStyleG = 0
    private var mFillStyleB = 0
    private var mFillStyleA = 255
    private var mStrokeStyleR = 0
    private var mStrokeStyleG = 0
    private var mStrokeStyleB = 0
    private var mStrokeStyleA = 255
    private var mFontName = "Arial"
    private var mFontSize = 40.0f
    private var mLineWidth = 0.0f
    private var mIsBoldFont = false
    private var mIsItalicFont = false
    private var mIsObliqueFont = false
    private var mIsSmallCapsFontVariant = false
    private var mLineCap = "butt"
    private var mLineJoin = "miter"

    private inner class Size {
        internal constructor(w: Float, h: Float) {
            width = w
            height = h
        }

        internal constructor() {
            width = 0f
            height = 0f
        }

        var width: Float
        var height: Float
    }

    private inner class Point {
        internal constructor(x: Float, y: Float) {
            this.x = x
            this.y = y
        }

        internal constructor() {
            y = 0.0f
            x = y
        }

        internal constructor(pt: Point) {
            x = pt.x
            y = pt.y
        }

        operator fun set(x: Float, y: Float) {
            this.x = x
            this.y = y
        }

        var x: Float
        var y: Float
    }

    private fun recreateBuffer(w: Float, h: Float) {
        if (mBitmap != null) {
            mBitmap!!.recycle()
        }
        mBitmap = Bitmap.createBitmap(
            Math.ceil(w.toDouble()).toInt(),
            Math.ceil(h.toDouble()).toInt(),
            Bitmap.Config.ARGB_8888
        )
        mCanvas.setBitmap(mBitmap)
    }

    private fun beginPath() {
        if (mLinePath == null) {
            mLinePath = Path()
        }
        mLinePath!!.reset()
    }

    private fun closePath() {
        mLinePath!!.close()
    }

    private fun moveTo(x: Float, y: Float) {
        mLinePath!!.moveTo(x, y)
    }

    private fun lineTo(x: Float, y: Float) {
        mLinePath!!.lineTo(x, y)
    }

    private fun stroke() {
        if (mLinePaint == null) {
            mLinePaint = Paint()
            mLinePaint!!.isAntiAlias = true
        }
        if (mLinePath == null) {
            mLinePath = Path()
        }
        mLinePaint!!.setARGB(mStrokeStyleA, mStrokeStyleR, mStrokeStyleG, mStrokeStyleB)
        mLinePaint!!.style = Paint.Style.STROKE
        mLinePaint!!.strokeWidth = mLineWidth
        setStrokeCap(mLinePaint!!)
        setStrokeJoin(mLinePaint!!)
        mCanvas.drawPath(mLinePath!!, mLinePaint!!)
    }

    private fun setStrokeCap(paint: Paint) {
        when (mLineCap) {
            "butt" -> paint.strokeCap = Paint.Cap.BUTT
            "round" -> paint.strokeCap = Paint.Cap.ROUND
            "square" -> paint.strokeCap = Paint.Cap.SQUARE
        }
    }

    private fun setStrokeJoin(paint: Paint) {
        when (mLineJoin) {
            "bevel" -> paint.strokeJoin = Paint.Join.BEVEL
            "round" -> paint.strokeJoin = Paint.Join.ROUND
            "miter" -> paint.strokeJoin = Paint.Join.MITER
        }
    }

    private fun fill() {
        if (mLinePaint == null) {
            mLinePaint = Paint()
        }
        if (mLinePath == null) {
            mLinePath = Path()
        }
        mLinePaint!!.setARGB(mFillStyleA, mFillStyleR, mFillStyleG, mFillStyleB)
        mLinePaint!!.style = Paint.Style.FILL
        mCanvas.drawPath(mLinePath!!, mLinePaint!!)
        mLinePaint!!.strokeWidth = 0f
        setStrokeCap(mLinePaint!!)
        setStrokeJoin(mLinePaint!!)
        mLinePaint!!.style = Paint.Style.STROKE
        mCanvas.drawPath(mLinePath!!, mLinePaint!!)
        mLinePaint!!.strokeWidth = mLineWidth
    }

    private fun setLineCap(lineCap: String) {
        mLineCap = lineCap
    }

    private fun setLineJoin(lineJoin: String) {
        mLineJoin = lineJoin
    }

    private fun saveContext() {
        mCanvas.save()
    }

    private fun restoreContext() {
        if (mCanvas.saveCount > 1) {
            mCanvas.restore()
        }
    }

    private fun rect(x: Float, y: Float, w: Float, h: Float) {
        beginPath()
        moveTo(x, y)
        lineTo(x, y + h)
        lineTo(x + w, y + h)
        lineTo(x + w, y)
        closePath()
    }

    private fun clearRect(x: Float, y: Float, w: Float, h: Float) {
        val clearSize = (w * h).toInt()
        val clearColor = IntArray(clearSize)
        for (i in 0 until clearSize) {
            clearColor[i] = Color.TRANSPARENT
        }
        mBitmap!!.setPixels(clearColor, 0, w.toInt(), x.toInt(), y.toInt(), w.toInt(), h.toInt())
    }

    private fun createTextPaintIfNeeded() {
        if (mTextPaint == null) {
            mTextPaint = newPaint(
                mFontName,
                mFontSize.toInt(),
                mIsBoldFont,
                mIsItalicFont,
                mIsObliqueFont,
                mIsSmallCapsFontVariant
            )
        }
    }

    private fun fillRect(x: Float, y: Float, w: Float, h: Float) {
        val pixelValue =
            mFillStyleA and 0xff shl 24 or (mFillStyleR and 0xff shl 16) or (mFillStyleG and 0xff shl 8) or (mFillStyleB and 0xff)
        val fillSize = (w * h).toInt()
        val fillColors = IntArray(fillSize)
        for (i in 0 until fillSize) {
            fillColors[i] = pixelValue
        }
        mBitmap!!.setPixels(fillColors, 0, w.toInt(), x.toInt(), y.toInt(), w.toInt(), h.toInt())
    }

    private fun scaleX(textPaint: TextPaint?, text: String, maxWidth: Float) {
        if (maxWidth < Float.MIN_VALUE) return
        val measureWidth = measureText(text)
        if (measureWidth - maxWidth < Float.MIN_VALUE) return
        val scaleX = maxWidth / measureWidth
        textPaint!!.textScaleX = scaleX
    }

    private fun fillText(text: String, x: Float, y: Float, maxWidth: Float) {
        createTextPaintIfNeeded()
        mTextPaint!!.setARGB(mFillStyleA, mFillStyleR, mFillStyleG, mFillStyleB)
        mTextPaint!!.style = Paint.Style.FILL
        scaleX(mTextPaint, text, maxWidth)
        val pt = convertDrawPoint(Point(x, y), text)
        mCanvas.drawText(text, pt.x, pt.y, mTextPaint!!)
    }

    private fun strokeText(text: String, x: Float, y: Float, maxWidth: Float) {
        createTextPaintIfNeeded()
        mTextPaint!!.setARGB(mStrokeStyleA, mStrokeStyleR, mStrokeStyleG, mStrokeStyleB)
        mTextPaint!!.style = Paint.Style.STROKE
        mTextPaint!!.strokeWidth = mLineWidth
        scaleX(mTextPaint, text, maxWidth)
        val pt = convertDrawPoint(Point(x, y), text)
        mCanvas.drawText(text, pt.x, pt.y, mTextPaint!!)
    }

    private fun measureText(text: String): Float {
        createTextPaintIfNeeded()
        return mTextPaint!!.measureText(text)
    }

    private fun measureTextReturnSize(text: String): Size {
        createTextPaintIfNeeded()
        val fm = mTextPaint!!.fontMetrics
        return Size(measureText(text), fm.descent - fm.ascent)
    }

    private fun updateFont(
        fontName: String,
        fontSize: Float,
        bold: Boolean,
        italic: Boolean,
        oblique: Boolean,
        smallCaps: Boolean
    ) {
        mFontName = fontName
        mFontSize = fontSize
        mIsBoldFont = bold
        mIsItalicFont = italic
        mIsObliqueFont = oblique
        mIsSmallCapsFontVariant = smallCaps
        mTextPaint = null // Reset paint to re-create paint object in createTextPaintIfNeeded
    }

    private fun setTextAlign(align: Int) {
        mTextAlign = align
    }

    private fun setTextBaseline(baseline: Int) {
        mTextBaseline = baseline
    }

    private fun setFillStyle(r: Float, g: Float, b: Float, a: Float) {
        mFillStyleR = (r * 255.0f).toInt()
        mFillStyleG = (g * 255.0f).toInt()
        mFillStyleB = (b * 255.0f).toInt()
        mFillStyleA = (a * 255.0f).toInt()
    }

    private fun setStrokeStyle(r: Float, g: Float, b: Float, a: Float) {
        mStrokeStyleR = (r * 255.0f).toInt()
        mStrokeStyleG = (g * 255.0f).toInt()
        mStrokeStyleB = (b * 255.0f).toInt()
        mStrokeStyleA = (a * 255.0f).toInt()
    }

    private fun setLineWidth(lineWidth: Float) {
        mLineWidth = lineWidth
    }

    private fun _fillImageData(
        imageData: ByteArray,
        imageWidth: Float,
        imageHeight: Float,
        offsetX: Float,
        offsetY: Float
    ) {
        val fillSize = (imageWidth * imageHeight).toInt()
        val fillColors = IntArray(fillSize)
        var r: Int
        var g: Int
        var b: Int
        var a: Int
        for (i in 0 until fillSize) {
            r = imageData[4 * i + 0].toInt() and 0xff
            g = imageData[4 * i + 1].toInt() and 0xff
            b = imageData[4 * i + 2].toInt() and 0xff
            a = imageData[4 * i + 3].toInt() and 0xff
            fillColors[i] =
                a and 0xff shl 24 or (r and 0xff shl 16) or (g and 0xff shl 8) or (b and 0xff)
        }
        mBitmap!!.setPixels(
            fillColors,
            0,
            imageWidth.toInt(),
            offsetX.toInt(),
            offsetY.toInt(),
            imageWidth.toInt(),
            imageHeight.toInt()
        )
    }

    private fun convertDrawPoint(point: Point, text: String): Point {
        val ret: Point = Point(point)
        val textSize = measureTextReturnSize(text)
        if (mTextAlign == TEXT_ALIGN_CENTER) {
            ret.x -= textSize.width / 2
        } else if (mTextAlign == TEXT_ALIGN_RIGHT) {
            ret.x -= textSize.width
        }
        if (mTextBaseline == TEXT_BASELINE_TOP) {
            ret.y += textSize.height
        } else if (mTextBaseline == TEXT_BASELINE_MIDDLE) {
            ret.y += textSize.height / 2
        }
        return ret
    }

    private val dataRef: ByteArray?
        get() {
            if (mBitmap != null) {
                val len = mBitmap!!.width * mBitmap!!.height * 4
                val pixels = ByteArray(len)
                val buf = ByteBuffer.wrap(pixels)
                buf.order(ByteOrder.nativeOrder())
                mBitmap!!.copyPixelsToBuffer(buf)
                return pixels
            }
            return null
        }

    companion object {
        private const val TAG = "CanvasContext2D"
        private const val TEXT_ALIGN_LEFT = 0
        private const val TEXT_ALIGN_CENTER = 1
        private const val TEXT_ALIGN_RIGHT = 2
        private const val TEXT_BASELINE_TOP = 0
        private const val TEXT_BASELINE_MIDDLE = 1
        private const val TEXT_BASELINE_BOTTOM = 2
        private var sContext: WeakReference<Context?>? = null
        private const val _sApproximatingOblique = -0.25f //please check paint api documentation
        @JvmStatic
        fun init(context: Context?) {
            sContext = WeakReference(context)
        }

        @JvmStatic
        fun destroy() {
            sContext = null
        }

        private val sTypefaceCache = HashMap<String, Typeface>()

        @JvmStatic
        private fun loadTypeface(familyName: String, urls: String) {
            var url = urls
            if (!sTypefaceCache.containsKey(familyName)) {
                try {
                    var typeface: Typeface? = null
                    if (url.startsWith("/")) {
                        typeface = Typeface.createFromFile(url)
                    } else if (sContext!!.get() != null) {
                        val prefix = "@assets/"
                        if (url.startsWith(prefix)) {
                            url = url.substring(prefix.length)
                        }
                        typeface = Typeface.createFromAsset(
                            CocosAssetsMgr.getAssets(
                                sContext!!.get()!!
                            ), url
                        )
                    }
                    if (typeface != null) {
                        sTypefaceCache[familyName] = typeface
                    }
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
        }

        @JvmStatic
        private fun clearTypefaceCache() {
            sTypefaceCache.clear()
        }

        @JvmStatic
        private fun newPaint(
            fontName: String,
            fontSize: Int,
            enableBold: Boolean,
            enableItalic: Boolean,
            obliqueFont: Boolean,
            smallCapsFontVariant: Boolean
        ): TextPaint {
            val paint = TextPaint()
            paint.textSize = fontSize.toFloat()
            paint.isAntiAlias = true
            paint.isSubpixelText = true
            var key = fontName
            if (enableBold) {
                key += "-Bold"
                paint.isFakeBoldText = true
            }
            if (enableItalic) {
                key += "-Italic"
            }
            val typeFace: Typeface?
            if (sTypefaceCache.containsKey(key)) {
                typeFace = sTypefaceCache[key]
            } else {
                var style = Typeface.NORMAL
                if (enableBold && enableItalic) {
                    style = Typeface.BOLD_ITALIC
                } else if (enableBold) {
                    style = Typeface.BOLD
                } else if (enableItalic) {
                    style = Typeface.ITALIC
                }
                typeFace = Typeface.create(fontName, style)
            }
            paint.typeface = typeFace
            if (obliqueFont) {
                paint.textSkewX = _sApproximatingOblique
            }
            if (smallCapsFontVariant && Build.VERSION.SDK_INT >= 21) {
                Cocos2dxReflectionHelper.invokeInstanceMethod<Void>(
                    paint,
                    "setFontFeatureSettings",
                    arrayOf(String::class.java),
                    arrayOf("smcp")
                )
            }
            return paint
        }
    }
}
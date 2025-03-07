#include "platform/CCCanvasRenderingContext2D.h"
#include "base/ccTypes.h"
#include "base/csscolorparser.hpp"

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "cocos/scripting/js-bindings/manual/jsb_platform.h"

#include "platform/CCFileUtils.h"
#include <regex>

using namespace cocos2d;

enum class CanvasTextAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum class CanvasTextBaseline {
    TOP,
    MIDDLE,
    BOTTOM
};

namespace {
    void fillRectWithColor(uint8_t* buf, uint32_t totalWidth, uint32_t totalHeight, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        assert(x + width <= totalWidth);
        assert(y + height <= totalHeight);

        uint32_t y0 = totalHeight - (y + height);
        uint32_t y1 = totalHeight - y;
        uint8_t* p;
        for (uint32_t offsetY = y0; offsetY < y1; ++offsetY)
        {
            for (uint32_t offsetX = x; offsetX < (x + width); ++offsetX)
            {
                p = buf + (totalWidth * offsetY + offsetX) * 4;
                *p++ = r;
                *p++ = g;
                *p++ = b;
                *p++ = a;
            }
        }
    }
}

class CanvasRenderingContext2DImpl
{
public:
    CanvasRenderingContext2DImpl() : _DC(nullptr)
    , _bmp(nullptr)
    , _font((HFONT)GetStockObject(DEFAULT_GUI_FONT))
    , _wnd(nullptr)
    , _savedDC(0)
    {
        _wnd = nullptr;
        HDC hdc = GetDC(_wnd);
        _DC = CreateCompatibleDC(hdc);
        ReleaseDC(_wnd, hdc);
    }

    ~CanvasRenderingContext2DImpl()
    {
        _deleteBitmap();
        _removeCustomFont();
        if (_DC)
            DeleteDC(_DC);
    }

    void recreateBuffer(float w, float h)
    {
        _bufferWidth = w;
        _bufferHeight = h;
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
        {
            _deleteBitmap();
            return;
        }

        int textureSize = _bufferWidth * _bufferHeight * 4;
        uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * textureSize);
        memset(data, 0x00, textureSize);
        _imageData.fastSet(data, textureSize);

        _prepareBitmap(_bufferWidth, _bufferHeight);
    }

    void beginPath()
    {
        _hpen = CreatePen(PS_SOLID, _lineWidth, RGB(255, 255, 255));
        HGDIOBJ hOldPen = SelectObject(_DC, _hpen);
        HGDIOBJ hOldBmp = SelectObject(_DC, _bmp);
        DeleteObject(hOldPen);
        DeleteObject(hOldBmp);

        SetBkMode(_DC, TRANSPARENT);
    }

    void closePath()
    {
    }

    void moveTo(float x, float y)
    {
        MoveToEx(_DC, x, -(y - _bufferHeight - _fontSize), nullptr);
    }

    void lineTo(float x, float y)
    {
        LineTo(_DC, x, -(y - _bufferHeight - _fontSize));
    }

    void stroke()
    {
        DeleteObject(_hpen);
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        _fillTextureData();
    }

    void saveContext()
    {
        _savedDC = SaveDC(_DC);
    }

    void restoreContext()
    {
        BOOL ret = RestoreDC(_DC, _savedDC);
        if (0 == ret)
        {
            SE_LOGD("CanvasRenderingContext2DImpl restore context failed.\n");
        }
    }

    void clearRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        if (_imageData.isNull())
            return;

        recreateBuffer(w, h);
    }

    void fillRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        uint8_t* buffer = _imageData.getBytes();
        if (buffer)
        {
            uint8_t r = _fillStyle.r * 255.0f;
            uint8_t g = _fillStyle.g * 255.0f;
            uint8_t b = _fillStyle.b * 255.0f;
            uint8_t a = _fillStyle.a * 255.0f;
            fillRectWithColor(buffer, (uint32_t)_bufferWidth, (uint32_t)_bufferHeight, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h, r, g, b, a);
        }
    }

    void fillText(const std::string& text, float x, float y, float maxWidth)
    {
        if (text.empty() || _bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        SIZE textSize = { 0, 0 };
        Point offsetPoint = _convertDrawPoint(Point(x, y), text);

        _drawText(text, (int)offsetPoint.x, (int)offsetPoint.y);
        _fillTextureData();

    }

    void strokeText(const std::string& text, float x, float y, float maxWidth)
    {
        if (text.empty() || _bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
    }

    cocos2d::Size measureText(const std::string& text)
    {
        if (text.empty())
            return Size(0.0f, 0.0f);

        int bufferLen = 0;
        wchar_t * pwszBuffer = _utf8ToUtf16(text, &bufferLen);
        SIZE size = _sizeWithText(pwszBuffer, bufferLen);
        CC_SAFE_DELETE_ARRAY(pwszBuffer);
        return Size(size.cx, size.cy);
    }

    void updateFont(const std::string& fontName, float fontSize, bool bold = false)
    {
        do
        {
            _fontName = fontName;
            _fontSize = fontSize;
            std::string fontPath;
            LOGFONTA    tFont = { 0 };
            if (!_fontName.empty())
            {
                const auto& fontInfoMap = getFontFamilyNameMap();
                auto iter = fontInfoMap.find(_fontName);
                if (iter != fontInfoMap.end())
                {
                    fontPath = iter->second;
                    std::string tmpFontPath = fontPath;
                    int nFindPos = tmpFontPath.rfind("/");
                    tmpFontPath = &tmpFontPath[nFindPos + 1];
                    nFindPos = tmpFontPath.rfind(".");
                    _fontName = tmpFontPath.substr(0, nFindPos);
                }
                else
                {
                    auto nFindPos = fontName.rfind("/");
                    if (nFindPos != fontName.npos)
                    {
                        if (fontName.length() == nFindPos + 1)
                        {
                            _fontName = "";
                        }
                        else
                        {
                            _fontName = &_fontName[nFindPos + 1];
                        }
                    }
                }
                tFont.lfCharSet = DEFAULT_CHARSET;
                strcpy_s(tFont.lfFaceName, LF_FACESIZE, _fontName.c_str());
            }

            if (_fontSize)
                tFont.lfHeight = -_fontSize;

            if (bold)
                tFont.lfWeight = FW_BOLD;
            else
                tFont.lfWeight = FW_NORMAL;

            tFont.lfQuality = ANTIALIASED_QUALITY;

            _removeCustomFont();

            if (fontPath.size() > 0)
            {
                _curFontPath = fontPath;
                wchar_t * pwszBuffer = _utf8ToUtf16(_curFontPath);
                if (pwszBuffer)
                {
                    if (AddFontResource(pwszBuffer))
                    {
                        SendMessage(_wnd, WM_FONTCHANGE, 0, 0);
                    }
                    delete[] pwszBuffer;
                    pwszBuffer = nullptr;
                }
            }

            _font = CreateFontIndirectA(&tFont);
            if (!_font)
            {
                SE_LOGE("Failed to create custom font(font name: %s, font size: %f), use default font.\n",
                    _fontName.c_str(), fontSize);
                break;
            }
            else
            {
                SelectObject(_DC, _font);
                SendMessage(_wnd, WM_FONTCHANGE, 0, 0);
            }
        } while (0);
    }

    void setTextAlign(CanvasTextAlign align)
    {
        _textAlign = align;
    }

    void setTextBaseline(CanvasTextBaseline baseline)
    {
        _textBaseLine = baseline;
    }

    void setFillStyle(float r, float g, float b, float a)
    {
        _fillStyle.r = r;
        _fillStyle.g = g;
        _fillStyle.b = b;
        _fillStyle.a = a;
    }

    void setStrokeStyle(float r, float g, float b, float a)
    {
        _strokeStyle.r = r;
        _strokeStyle.g = g;
        _strokeStyle.b = b;
        _strokeStyle.a = a;
    }

    void setLineWidth(float lineWidth)
    {
        _lineWidth = lineWidth;
    }

    const Data& getDataRef() const
    {
        return _imageData;
    }

    HDC _DC;
    HBITMAP _bmp;
private:

    Data _imageData;
    HFONT   _font;
    HWND    _wnd;
    HPEN _hpen;
    PAINTSTRUCT _paintStruct;
    std::string _curFontPath;
    int _savedDC;
    float _lineWidth = 0.0f;
    float _bufferWidth = 0.0f;
    float _bufferHeight = 0.0f;

    std::string _fontName;
    int _fontSize;
    SIZE _textSize;
    CanvasTextAlign _textAlign;
    CanvasTextBaseline _textBaseLine;
    cocos2d::Color4F _fillStyle;
    cocos2d::Color4F _strokeStyle;

    TEXTMETRIC _tm;

    wchar_t * _utf8ToUtf16(const std::string& str, int * pRetLen = nullptr)
    {
        wchar_t * pwszBuffer = nullptr;
        do
        {
            if (str.empty())
            {
                break;
            }
            int nLen = str.size();
            int nBufLen = nLen + 1;
            pwszBuffer = new wchar_t[nBufLen];
            CC_BREAK_IF(!pwszBuffer);
            memset(pwszBuffer, 0, sizeof(wchar_t) * nBufLen);
            int actuallyLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), nLen, pwszBuffer, nBufLen);
            if (pRetLen != nullptr) {
                *pRetLen = actuallyLen;
            }
        } while (0);
        return pwszBuffer;

    }

    void _removeCustomFont()
    {
        HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hDefFont != _font)
        {
            DeleteObject(SelectObject(_DC, hDefFont));
        }
        if (_curFontPath.size() > 0)
        {
            wchar_t * pwszBuffer = _utf8ToUtf16(_curFontPath);
            if (pwszBuffer)
            {
                RemoveFontResource(pwszBuffer);
                SendMessage(_wnd, WM_FONTCHANGE, 0, 0);
                delete[] pwszBuffer;
                pwszBuffer = nullptr;
            }
            _curFontPath.clear();
        }
    }

    int _drawText(const std::string& text, int x, int y)
    {
        int nRet = 0;
        wchar_t * pwszBuffer = nullptr;
        do
        {
            CC_BREAK_IF(text.empty());

            DWORD dwFmt = DT_SINGLELINE;

            int bufferLen = 0;
            pwszBuffer = _utf8ToUtf16(text, &bufferLen);

            SIZE newSize = _sizeWithText(pwszBuffer, bufferLen);

            _textSize = newSize;

            RECT rcText = { 0 };

            rcText.right = newSize.cx;
            rcText.bottom = newSize.cy;

            LONG offsetX = x;
            LONG offsetY = y;
            if (offsetX || offsetY)
            {
                OffsetRect(&rcText, offsetX, offsetY);
            }


            SetBkMode(_DC, TRANSPARENT);
            SetTextColor(_DC, RGB(255, 255, 255)); // white color

            nRet = DrawTextW(_DC, pwszBuffer, bufferLen, &rcText, dwFmt);
        } while (0);
        CC_SAFE_DELETE_ARRAY(pwszBuffer);

        return nRet;
    }

    SIZE _sizeWithText(const wchar_t * pszText, int nLen)
    {
        SIZE tRet = { 0 };
        do
        {
            CC_BREAK_IF(!pszText || nLen <= 0);

            RECT rc = { 0, 0, 0, 0 };
            DWORD dwCalcFmt = DT_CALCRECT;

            DrawTextW(_DC, pszText, nLen, &rc, dwCalcFmt);

            tRet.cx = rc.right;
            tRet.cy = rc.bottom;
        } while (0);

        return tRet;
    }

    void _prepareBitmap(int nWidth, int nHeight)
    {
        _deleteBitmap();

        if (nWidth > 0 && nHeight > 0)
        {
            _bmp = CreateBitmap(nWidth, nHeight, 1, 32, nullptr);
            SelectObject(_DC, _bmp);
        }
    }

    void _deleteBitmap()
    {
        if (_bmp)
        {
            DeleteObject(_bmp);
            _bmp = nullptr;
        }
    }

    void _fillTextureData()
    {
        do
        {
            int dataLen = _bufferWidth * _bufferHeight * 4;
            unsigned char* dataBuf = (unsigned char*)malloc(sizeof(unsigned char) * dataLen);
            CC_BREAK_IF(!dataBuf);
            unsigned char* imageBuf = _imageData.getBytes();
            CC_BREAK_IF(!imageBuf);

            struct
            {
                BITMAPINFOHEADER bmiHeader;
                int mask[4];
            } bi = { 0 };
            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            CC_BREAK_IF(!GetDIBits(_DC, _bmp, 0, 0,
                                   nullptr, (LPBITMAPINFO)&bi, DIB_RGB_COLORS));

            bi.bmiHeader.biHeight = (bi.bmiHeader.biHeight > 0) ? -bi.bmiHeader.biHeight : bi.bmiHeader.biHeight;
            GetDIBits(_DC, _bmp, 0, _bufferHeight, dataBuf,
                      (LPBITMAPINFO)&bi, DIB_RGB_COLORS);

            uint8_t r = _fillStyle.r * 255;
            uint8_t g = _fillStyle.g * 255;
            uint8_t b = _fillStyle.b * 255;
            COLORREF textColor = (b << 16 | g << 8 | r) & 0x00ffffff;
            COLORREF * pPixel = nullptr;
            COLORREF * pImage = nullptr;
            for (int y = 0; y < _bufferHeight; ++y)
            {
                pPixel = (COLORREF *)dataBuf + y * (int)_bufferWidth;
                pImage = (COLORREF *)imageBuf + y * (int)_bufferWidth;
                for (int x = 0; x < _bufferWidth; ++x)
                {
                    COLORREF& clr = *pPixel;
                    COLORREF& val = *pImage;
                    uint8_t alpha = GetRValue(clr);
                   
                    val = (alpha << 24) | textColor;

                    ++pPixel;
                    ++pImage;
                }
            }

            free(dataBuf);
        } while (0);
    }

    Point _convertDrawPoint(Point point, std::string text) {
        Size textSize = measureText(text);
        if (_textAlign == CanvasTextAlign::CENTER)
        {
            point.x -= textSize.width / 2.0f;
        }
        else if (_textAlign == CanvasTextAlign::RIGHT)
        {
            point.x -= textSize.width;
        }

        if (_textBaseLine == CanvasTextBaseline::TOP)
        {
            point.y += _fontSize;
        }
        else if (_textBaseLine == CanvasTextBaseline::MIDDLE)
        {
            point.y += _fontSize / 2.0f;
        }

        GetTextMetrics(_DC, &_tm);
        point.y -= _tm.tmAscent;

        return point;
    }
};

NS_CC_BEGIN

CanvasGradient::CanvasGradient()
{
}

CanvasGradient::~CanvasGradient()
{
}

void CanvasGradient::addColorStop(float offset, const std::string& color)
{
}


CanvasRenderingContext2D::CanvasRenderingContext2D(float width, float height)
: __width(width)
, __height(height)
{
    _impl = new CanvasRenderingContext2DImpl();
    recreateBufferIfNeeded();
}

CanvasRenderingContext2D::~CanvasRenderingContext2D()
{
    delete _impl;
}

void CanvasRenderingContext2D::recreateBufferIfNeeded()
{
    if (_isBufferSizeDirty)
    {
        _isBufferSizeDirty = false;
        _impl->recreateBuffer(__width, __height);
        if (_canvasBufferUpdatedCB != nullptr)
            _canvasBufferUpdatedCB(_impl->getDataRef());
    }
}

void CanvasRenderingContext2D::clearRect(float x, float y, float width, float height)
{
    recreateBufferIfNeeded();
    _impl->clearRect(x, y, width, height);
}

void CanvasRenderingContext2D::fillRect(float x, float y, float width, float height)
{
    recreateBufferIfNeeded();
    _impl->fillRect(x, y, width, height);

    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::fillText(const std::string& text, float x, float y, float maxWidth)
{
    if (text.empty())
        return;
    recreateBufferIfNeeded();

    _impl->fillText(text, x, y, maxWidth);
    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::strokeText(const std::string& text, float x, float y, float maxWidth)
{
    if (text.empty())
        return;
    recreateBufferIfNeeded();

    _impl->strokeText(text, x, y, maxWidth);

    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

cocos2d::Size CanvasRenderingContext2D::measureText(const std::string& text)
{
    return _impl->measureText(text);
}

CanvasGradient* CanvasRenderingContext2D::createLinearGradient(float x0, float y0, float x1, float y1)
{
    return nullptr;
}

void CanvasRenderingContext2D::save()
{
    _impl->saveContext();
}

void CanvasRenderingContext2D::beginPath()
{
    _impl->beginPath();
}

void CanvasRenderingContext2D::closePath()
{
    _impl->closePath();
}

void CanvasRenderingContext2D::moveTo(float x, float y)
{
    _impl->moveTo(x, y);
}

void CanvasRenderingContext2D::lineTo(float x, float y)
{
    _impl->lineTo(x, y);
}

void CanvasRenderingContext2D::stroke()
{
    _impl->stroke();

    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::restore()
{
    _impl->restoreContext();
}

void CanvasRenderingContext2D::setCanvasBufferUpdatedCallback(const CanvasBufferUpdatedCallback& cb)
{
    _canvasBufferUpdatedCB = cb;
}

void CanvasRenderingContext2D::set__width(float width)
{
    __width = width;
    _isBufferSizeDirty = true;
    recreateBufferIfNeeded();
}

void CanvasRenderingContext2D::set__height(float height)
{
    __height = height;
    _isBufferSizeDirty = true;
    recreateBufferIfNeeded();
}

void CanvasRenderingContext2D::set_lineWidth(float lineWidth)
{
    _lineWidth = lineWidth;
    _impl->setLineWidth(lineWidth);
}

void CanvasRenderingContext2D::set_lineCap(const std::string& lineCap)
{
}

void CanvasRenderingContext2D::set_lineJoin(const std::string& lineJoin)
{
}

void CanvasRenderingContext2D::fill()
{
}
void CanvasRenderingContext2D::rect(float x, float y, float w, float h)
{
}

void CanvasRenderingContext2D::set_font(const std::string& font)
{
    if (_font != font)
    {
        _font = font;

        std::string boldStr;
        std::string fontName = "Arial";
        std::string fontSizeStr = "30";

        std::regex re("(bold)?\\s*((\\d+)([\\.]\\d+)?)px\\s+([\\w-]+|\"[\\w -]+\"$)");
        std::match_results<std::string::const_iterator> results;
        if (std::regex_search(_font.cbegin(), _font.cend(), results, re))
        {
            boldStr = results[1].str();
            fontSizeStr = results[2].str();
            fontName = results[5].str();
        }

        float fontSize = atof(fontSizeStr.c_str());
        _impl->updateFont(fontName, fontSize, !boldStr.empty());
    }
}

void CanvasRenderingContext2D::set_textAlign(const std::string& textAlign)
{
    if (textAlign == "left")
    {
        _impl->setTextAlign(CanvasTextAlign::LEFT);
    }
    else if (textAlign == "center" || textAlign == "middle")
    {
        _impl->setTextAlign(CanvasTextAlign::CENTER);
    }
    else if (textAlign == "right")
    {
        _impl->setTextAlign(CanvasTextAlign::RIGHT);
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_textBaseline(const std::string& textBaseline)
{
    if (textBaseline == "top")
    {
        _impl->setTextBaseline(CanvasTextBaseline::TOP);
    }
    else if (textBaseline == "middle")
    {
        _impl->setTextBaseline(CanvasTextBaseline::MIDDLE);
    }
    else if (textBaseline == "bottom" || textBaseline == "alphabetic") //REFINE:, how to deal with alphabetic, currently we handle it as bottom mode.
    {
        _impl->setTextBaseline(CanvasTextBaseline::BOTTOM);
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_fillStyle(const std::string& fillStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(fillStyle);
    _impl->setFillStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
}

void CanvasRenderingContext2D::set_strokeStyle(const std::string& strokeStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(strokeStyle);
    _impl->setStrokeStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
}

void CanvasRenderingContext2D::set_globalCompositeOperation(const std::string& globalCompositeOperation)
{
}

void CanvasRenderingContext2D::_fillImageData(const Data& imageData, float imageWidth, float imageHeight, float offsetX, float offsetY)
{
}

void CanvasRenderingContext2D::translate(float x, float y)
{
}

void CanvasRenderingContext2D::scale(float x, float y)
{
}

void CanvasRenderingContext2D::rotate(float angle)
{
}

void CanvasRenderingContext2D::transform(float a, float b, float c, float d, float e, float f)
{
}

void CanvasRenderingContext2D::setTransform(float a, float b, float c, float d, float e, float f)
{
}

NS_CC_END

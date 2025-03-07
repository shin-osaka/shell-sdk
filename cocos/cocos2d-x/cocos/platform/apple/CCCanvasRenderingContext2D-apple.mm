#include "platform/CCCanvasRenderingContext2D.h"
#include "base/ccTypes.h"
#include "base/csscolorparser.hpp"

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "cocos/scripting/js-bindings/manual/jsb_platform.h"

#import <Foundation/Foundation.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#import <Cocoa/Cocoa.h>
#else
#import <CoreText/CoreText.h>

#define NSBezierPath UIBezierPath
#define NSFont UIFont
#define NSColor UIColor
#define NSSize CGSize
#define NSZeroSize CGSizeZero
#define NSPoint CGPoint
#define NSMakePoint CGPointMake

#endif

#include <regex>
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

@interface CanvasRenderingContext2DImpl : NSObject {
    NSFont* _font;
    NSMutableDictionary* _tokenAttributesDict;
    NSString* _fontName;
    CGFloat _fontSize;
    CGFloat _width;
    CGFloat _height;
    CGContextRef _context;
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    NSGraphicsContext* _currentGraphicsContext;
    NSGraphicsContext* _oldGraphicsContext;
#else
    CGContextRef _oldContext;
#endif
    
    CGColorSpaceRef _colorSpace;
    cocos2d::Data _imageData;
    NSBezierPath* _path;

    CanvasTextAlign _textAlign;
    CanvasTextBaseline _textBaseLine;
    cocos2d::Color4F _fillStyle;
    cocos2d::Color4F _strokeStyle;
    float _lineWidth;
    bool _bold;
}

@property (nonatomic, strong) NSFont* font;
@property (nonatomic, strong) NSMutableDictionary* tokenAttributesDict;
@property (nonatomic, strong) NSString* fontName;
@property (nonatomic, assign) CanvasTextAlign textAlign;
@property (nonatomic, assign) CanvasTextBaseline textBaseLine;
@property (nonatomic, assign) float lineWidth;

@end

@implementation CanvasRenderingContext2DImpl

@synthesize font = _font;
@synthesize tokenAttributesDict = _tokenAttributesDict;
@synthesize fontName = _fontName;
@synthesize textAlign = _textAlign;
@synthesize textBaseLine = _textBaseLine;
@synthesize lineWidth = _lineWidth;

-(id) init {
    if (self = [super init]) {
        _lineWidth = 0;
        _textAlign = CanvasTextAlign::LEFT;
        _textBaseLine = CanvasTextBaseline::BOTTOM;
        _width = _height = 0;
        _context = nil;
        _colorSpace = nil;

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
        _currentGraphicsContext = nil;
        _oldGraphicsContext = nil;
#endif
        _path = [NSBezierPath bezierPath];
        [_path retain];
        [self updateFontWithName:@"Arial" fontSize:30 bold:false];
    }

    return self;
}

-(void) dealloc {
    self.font = nil;
    self.tokenAttributesDict = nil;
    self.fontName = nil;
    CGColorSpaceRelease(_colorSpace);
    CGContextRelease(_context);
    [_path release];
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    [_currentGraphicsContext release];
#endif
    [super dealloc];
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC

-(NSFont*) _createSystemFont {
    NSFontTraitMask mask = NSUnitalicFontMask;
    if (_bold) {
        mask |= NSBoldFontMask;
    }
    else {
        mask |= NSUnboldFontMask;
    }

    NSFont* font = [[NSFontManager sharedFontManager]
                    fontWithFamily:_fontName
                    traits:mask
                    weight:0
                    size:_fontSize];

    if (font == nil) {
        const auto& familyMap = getFontFamilyNameMap();
        auto iter = familyMap.find([_fontName UTF8String]);
        if (iter != familyMap.end()) {
            font = [[NSFontManager sharedFontManager]
               fontWithFamily: [NSString stringWithUTF8String:iter->second.c_str()]
               traits: mask
               weight: 0
               size: _fontSize];
        }
    }

    if (font == nil) {
        font = [[NSFontManager sharedFontManager]
                fontWithFamily: @"Arial"
                traits: mask
                weight: 0
                size: _fontSize];
    }
    return font;
}

#else

-(UIFont*) _createSystemFont {
    UIFont* font = nil;

    if (_bold) {
        font = [UIFont fontWithName:[_fontName stringByAppendingString:@"-Bold"] size:_fontSize];
    }
    else {
        font = [UIFont fontWithName:_fontName size:_fontSize];
    }

    if (font == nil) {
        const auto& familyMap = getFontFamilyNameMap();
        auto iter = familyMap.find([_fontName UTF8String]);
        if (iter != familyMap.end()) {
            font = [UIFont fontWithName:[NSString stringWithUTF8String:iter->second.c_str()] size:_fontSize];
        }
    }

    if (font == nil) {
        if (_bold) {
            font = [UIFont boldSystemFontOfSize:_fontSize];
        } else {
            font = [UIFont systemFontOfSize:_fontSize];
        }
    }
    return font;
}

#endif

-(void) updateFontWithName: (NSString*)fontName fontSize: (CGFloat)fontSize bold: (bool)bold{
    _fontSize = fontSize;
    _bold = bold;

    self.fontName = fontName;
    self.font = [self _createSystemFont];

    NSMutableParagraphStyle* paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    [paragraphStyle setAlignment:NSTextAlignmentCenter];

    NSColor* foregroundColor = [NSColor colorWithRed:1.0f
                                               green:1.0f
                                                blue:1.0f
                                               alpha:1.0f];

    self.tokenAttributesDict = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                                foregroundColor, NSForegroundColorAttributeName,
                                                _font, NSFontAttributeName,
                                                paragraphStyle, NSParagraphStyleAttributeName, nil];
}

-(void) recreateBufferWithWidth:(NSInteger) width height:(NSInteger) height {
    _width = width = width > 0 ? width : 1;
    _height = height = height > 0 ? height : 1;
    NSUInteger textureSize = width * height * 4;
    unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * textureSize);
    memset(data, 0, textureSize);
    _imageData.fastSet(data, textureSize);

    if (_context != nil)
    {
        CGContextRelease(_context);
        _context = nil;
    }

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    if (_currentGraphicsContext != nil)
    {
        [_currentGraphicsContext release];
        _currentGraphicsContext = nil;
    }
#endif

    _colorSpace  = CGColorSpaceCreateDeviceRGB();
    _context = CGBitmapContextCreate(data,
                                     width,
                                     height,
                                     8,
                                     width * 4,
                                     _colorSpace,
                                     kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    if (nil == _context)
    {
        CGColorSpaceRelease(_colorSpace); //REFINE: HOWTO RELEASE?
        _colorSpace = nil;
    }

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    _currentGraphicsContext = [NSGraphicsContext graphicsContextWithCGContext:_context flipped: NO];
    [_currentGraphicsContext retain];
#else
    CGContextTranslateCTM(_context, 0.0f, _height);

    CGContextScaleCTM(_context, 1.0f, -1.0f);
#endif
}

-(NSSize) measureText:(NSString*) text {

    NSAttributedString* stringWithAttributes = [[[NSAttributedString alloc] initWithString:text
                                                             attributes:_tokenAttributesDict] autorelease];

    NSSize textRect = NSZeroSize;
    textRect.width = CGFLOAT_MAX;
    textRect.height = CGFLOAT_MAX;

    NSSize dim = [stringWithAttributes boundingRectWithSize:textRect options:(NSStringDrawingOptions)(NSStringDrawingUsesLineFragmentOrigin) context:nil].size;

    dim.width = ceilf(dim.width);
    dim.height = ceilf(dim.height);

    return dim;
}

-(NSPoint) convertDrawPoint:(NSPoint) point text:(NSString*) text {
    NSSize textSize = [self measureText:text];

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

#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    point.y -= _font.descender;

    point.y = _height - point.y;
#else
    point.y -= _font.ascender;
#endif
    return point;
}

-(void) fillText:(NSString*) text x:(CGFloat) x y:(CGFloat) y maxWidth:(CGFloat) maxWidth {
    if (text.length == 0)
        return;

    NSPoint drawPoint = [self convertDrawPoint:NSMakePoint(x, y) text:text];

    NSMutableParagraphStyle* paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;

    [_tokenAttributesDict removeObjectForKey:NSStrokeColorAttributeName];

    [_tokenAttributesDict setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
    [_tokenAttributesDict setObject:[NSColor colorWithRed:_fillStyle.r green:_fillStyle.g blue:_fillStyle.b alpha:_fillStyle.a]
                             forKey:NSForegroundColorAttributeName];

    [self saveContext];

    CGContextSetRGBFillColor(_context, _fillStyle.r, _fillStyle.g, _fillStyle.b, _fillStyle.a);
    CGContextSetShouldSubpixelQuantizeFonts(_context, false);
    CGContextBeginTransparencyLayerWithRect(_context, CGRectMake(0, 0, _width, _height), nullptr);
    CGContextSetTextDrawingMode(_context, kCGTextFill);

    

    NSAttributedString *stringWithAttributes =[[[NSAttributedString alloc] initWithString:text
                                                                               attributes:_tokenAttributesDict] autorelease];

    [stringWithAttributes drawAtPoint:drawPoint];


    CGContextEndTransparencyLayer(_context);

    [self restoreContext];
}

-(void) strokeText:(NSString*) text x:(CGFloat) x y:(CGFloat) y maxWidth:(CGFloat) maxWidth {
    if (text.length == 0)
        return;

    NSPoint drawPoint = [self convertDrawPoint:NSMakePoint(x, y) text:text];

    NSMutableParagraphStyle* paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;

    [_tokenAttributesDict removeObjectForKey:NSForegroundColorAttributeName];

    [_tokenAttributesDict setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
    [_tokenAttributesDict setObject:[NSColor colorWithRed:_strokeStyle.r
                                                    green:_strokeStyle.g
                                                     blue:_strokeStyle.b
                                                    alpha:_strokeStyle.a] forKey:NSStrokeColorAttributeName];

    [self saveContext];

    CGContextSetRGBStrokeColor(_context, _strokeStyle.r, _strokeStyle.g, _strokeStyle.b, _strokeStyle.a);
    CGContextSetRGBFillColor(_context, _fillStyle.r, _fillStyle.g, _fillStyle.b, _fillStyle.a);
    CGContextSetLineWidth(_context, _lineWidth);
    CGContextSetLineJoin(_context, kCGLineJoinRound);
    CGContextSetShouldSubpixelQuantizeFonts(_context, false);
    CGContextBeginTransparencyLayerWithRect(_context, CGRectMake(0, 0, _width, _height), nullptr);

    CGContextSetTextDrawingMode(_context, kCGTextStroke);

    NSAttributedString *stringWithAttributes =[[[NSAttributedString alloc] initWithString:text
                                                                               attributes:_tokenAttributesDict] autorelease];

    [stringWithAttributes drawAtPoint:drawPoint];

    CGContextEndTransparencyLayer(_context);

    [self restoreContext];
}

-(void) setFillStyleWithRed:(CGFloat) r green:(CGFloat) g blue:(CGFloat) b alpha:(CGFloat) a {
    _fillStyle.r = r;
    _fillStyle.g = g;
    _fillStyle.b = b;
    _fillStyle.a = a;
}

-(void) setStrokeStyleWithRed:(CGFloat) r green:(CGFloat) g blue:(CGFloat) b alpha:(CGFloat) a {
    _strokeStyle.r = r;
    _strokeStyle.g = g;
    _strokeStyle.b = b;
    _strokeStyle.a = a;
}

-(const cocos2d::Data&) getDataRef {
    return _imageData;
}

-(void) clearRect:(CGRect) rect {
    if (_imageData.isNull())
        return;

    rect.origin.x = floor(rect.origin.x);
    rect.origin.y = floor(rect.origin.y);
    rect.size.width = floor(rect.size.width);
    rect.size.height = floor(rect.size.height);

    if (rect.origin.x < 0) rect.origin.x = 0;
    if (rect.origin.y < 0) rect.origin.y = 0;

    if (rect.size.width < 1 || rect.size.height < 1)
        return;
    memset((void*)_imageData.getBytes(), 0x00, _imageData.getSize());
}

-(void) fillRect:(CGRect) rect {
    [self saveContext];

    NSColor* color = [NSColor colorWithRed:_fillStyle.r green:_fillStyle.g blue:_fillStyle.b alpha:_fillStyle.a];
    [color setFill];
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    CGRect tmpRect = CGRectMake(rect.origin.x, _height - rect.origin.y - rect.size.height, rect.size.width, rect.size.height);
    [NSBezierPath fillRect:tmpRect];
#else
    NSBezierPath* path = [NSBezierPath bezierPathWithRect:rect];
    [path fill];
#endif
    [self restoreContext];
}

-(void) saveContext {
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    _oldGraphicsContext = [NSGraphicsContext currentContext];
    [NSGraphicsContext setCurrentContext:_currentGraphicsContext];
    [NSGraphicsContext saveGraphicsState];
    [[NSGraphicsContext currentContext] setShouldAntialias:YES];
#else
    _oldContext = UIGraphicsGetCurrentContext();
    UIGraphicsPushContext(_context);
    CGContextSaveGState(_context);
#endif
}

-(void) restoreContext {
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    [NSGraphicsContext restoreGraphicsState];
    [NSGraphicsContext setCurrentContext:_oldGraphicsContext];
    _oldGraphicsContext = nil;
#else
    CGContextRestoreGState(_context);
    UIGraphicsPopContext();
    _oldContext = nil;
#endif
}

-(void) beginPath {

}

-(void) stroke {
    NSColor* color = [NSColor colorWithRed:_strokeStyle.r green:_strokeStyle.g blue:_strokeStyle.b alpha:_strokeStyle.a];
    [color setStroke];
    [_path setLineWidth: _lineWidth];
    [_path stroke];
}

-(void) moveToX: (float) x y:(float) y {
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    [_path moveToPoint: NSMakePoint(x, _height - y)];
#else
    [_path moveToPoint: NSMakePoint(x, y)];
#endif
}

-(void) lineToX: (float) x y:(float) y {
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    [_path lineToPoint: NSMakePoint(x, _height - y)];
#else
    [_path addLineToPoint: NSMakePoint(x, y)];
#endif
}

@end

NS_CC_BEGIN

CanvasGradient::CanvasGradient()
{
    SE_LOGD("CanvasGradient constructor: %p\n", this);
}

CanvasGradient::~CanvasGradient()
{
    SE_LOGD("CanvasGradient destructor: %p\n", this);
}

void CanvasGradient::addColorStop(float offset, const std::string& color)
{
    SE_LOGD("CanvasGradient::addColorStop: %p\n", this);
}


namespace
{
#define CLAMP(V, HI) std::min( (V), (HI) )
    void unMultiplyAlpha(unsigned char* ptr, ssize_t size)
    {
        float alpha;
        for (int i = 0; i < size; i += 4)
        {
            alpha = (float)ptr[i + 3];
            if (alpha > 0)
            {
                ptr[i] = CLAMP((int)((float)ptr[i] / alpha * 255), 255);
                ptr[i+1] = CLAMP((int)((float)ptr[i+1] / alpha * 255), 255);
                ptr[i+2] =  CLAMP((int)((float)ptr[i+2] / alpha * 255), 255);
            }
        }
    }
}

#define SEND_DATA_TO_JS(CB, IMPL) \
if (CB) \
{ \
    Data data([IMPL getDataRef]); \
    unMultiplyAlpha(data.getBytes(), data.getSize() ); \
    CB(data); \
}

CanvasRenderingContext2D::CanvasRenderingContext2D(float width, float height)
: __width(width)
, __height(height)
{
    _impl = [[CanvasRenderingContext2DImpl alloc] init];
    [_impl recreateBufferWithWidth:width height:height];
}

CanvasRenderingContext2D::~CanvasRenderingContext2D()
{
    [_impl release];
}

void CanvasRenderingContext2D::recreateBufferIfNeeded()
{
    if (_isBufferSizeDirty)
    {
        _isBufferSizeDirty = false;
        [_impl recreateBufferWithWidth: __width height:__height];
        SEND_DATA_TO_JS(_canvasBufferUpdatedCB, _impl);
    }
}

void CanvasRenderingContext2D::clearRect(float x, float y, float width, float height)
{
    recreateBufferIfNeeded();
    [_impl clearRect:CGRectMake(x, y, width, height)];
}

void CanvasRenderingContext2D::fillRect(float x, float y, float width, float height)
{
    recreateBufferIfNeeded();
    [_impl fillRect:CGRectMake(x, y, width, height)];
    SEND_DATA_TO_JS(_canvasBufferUpdatedCB, _impl);
}

void CanvasRenderingContext2D::fillText(const std::string& text, float x, float y, float maxWidth)
{
    if (text.empty())
        return;

    recreateBufferIfNeeded();

    [_impl fillText:[NSString stringWithUTF8String:text.c_str()] x:x y:y maxWidth:maxWidth];
    SEND_DATA_TO_JS(_canvasBufferUpdatedCB, _impl);
}

void CanvasRenderingContext2D::strokeText(const std::string& text, float x, float y, float maxWidth)
{
    if (text.empty())
        return;
    recreateBufferIfNeeded();

    [_impl strokeText:[NSString stringWithUTF8String:text.c_str()] x:x y:y maxWidth:maxWidth];
    SEND_DATA_TO_JS(_canvasBufferUpdatedCB, _impl);
}

cocos2d::Size CanvasRenderingContext2D::measureText(const std::string& text)
{
    CGSize size = [_impl measureText: [NSString stringWithUTF8String:text.c_str()]];
    return cocos2d::Size(size.width, size.height);
}

CanvasGradient* CanvasRenderingContext2D::createLinearGradient(float x0, float y0, float x1, float y1)
{
    return nullptr;
}

void CanvasRenderingContext2D::save()
{
    [_impl saveContext];
}

void CanvasRenderingContext2D::beginPath()
{
    [_impl beginPath];
}

void CanvasRenderingContext2D::closePath()
{
}

void CanvasRenderingContext2D::moveTo(float x, float y)
{
    [_impl moveToX:x y:y];
}

void CanvasRenderingContext2D::lineTo(float x, float y)
{
    [_impl lineToX:x y:y];
}

void CanvasRenderingContext2D::stroke()
{
    [_impl stroke];
    SEND_DATA_TO_JS(_canvasBufferUpdatedCB, _impl);
}

void CanvasRenderingContext2D::fill()
{
}

void CanvasRenderingContext2D::rect(float x, float y, float w, float h)
{
}

void CanvasRenderingContext2D::restore()
{
    [_impl restoreContext];
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
    _impl.lineWidth = _lineWidth;
}

void CanvasRenderingContext2D::set_lineCap(const std::string& lineCap)
{
}

void CanvasRenderingContext2D::set_lineJoin(const std::string& lineJoin)
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

        CGFloat fontSize = atof(fontSizeStr.c_str());
        [_impl updateFontWithName:[NSString stringWithUTF8String:fontName.c_str()] fontSize:fontSize bold:!boldStr.empty()];
    }
}

void CanvasRenderingContext2D::set_textAlign(const std::string& textAlign)
{
    if (textAlign == "left")
    {
        _impl.textAlign = CanvasTextAlign::LEFT;
    }
    else if (textAlign == "center" || textAlign == "middle")
    {
        _impl.textAlign = CanvasTextAlign::CENTER;
    }
    else if (textAlign == "right")
    {
        _impl.textAlign = CanvasTextAlign::RIGHT;
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
        _impl.textBaseLine = CanvasTextBaseline::TOP;
    }
    else if (textBaseline == "middle")
    {
        _impl.textBaseLine = CanvasTextBaseline::MIDDLE;
    }
    else if (textBaseline == "bottom" || textBaseline == "alphabetic") //REFINE:, how to deal with alphabetic, currently we handle it as bottom mode.
    {
        _impl.textBaseLine = CanvasTextBaseline::BOTTOM;
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_fillStyle(const std::string& fillStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(fillStyle);
    [_impl setFillStyleWithRed:color.r/255.0f green:color.g/255.0f blue:color.b/255.0f alpha:color.a];
}

void CanvasRenderingContext2D::set_strokeStyle(const std::string& strokeStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(strokeStyle);
    [_impl setStrokeStyleWithRed:color.r/255.0f green:color.g/255.0f blue:color.b/255.0f alpha:color.a];
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

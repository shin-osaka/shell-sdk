/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebGLTexelConversions.h"
#include <memory.h>

namespace mozilla {

using namespace WebGLTexelConversions;

namespace {

/** @class WebGLImageConverter
 *
 * This class is just a helper to implement WebGLContext::ConvertImage below.
 *
 * Design comments:
 *
 * WebGLContext::ConvertImage has to handle hundreds of format conversion paths.
 * It is important to minimize executable code size here. Instead of passing around
 * a large number of function parameters hundreds of times, we create a
 * WebGLImageConverter object once, storing these parameters, and then we call
 * the run() method on it.
 */
class WebGLImageConverter
{
    const size_t mWidth, mHeight;
    const void* const mSrcStart;
    void* const mDstStart;
    const ptrdiff_t mSrcStride, mDstStride;
    bool mAlreadyRun;
    bool mSuccess;

    /*
     * Returns sizeof(texel)/sizeof(type). The point is that we will iterate over
     * texels with typed pointers and this value will tell us by how much we need
     * to increment these pointers to advance to the next texel.
     */
    template<WebGLTexelFormat Format>
    static size_t NumElementsPerTexelForFormat() {
        switch (Format) {
            case WebGLTexelFormat::A8:
            case WebGLTexelFormat::A16F:
            case WebGLTexelFormat::A32F:
            case WebGLTexelFormat::R8:
            case WebGLTexelFormat::R16F:
            case WebGLTexelFormat::R32F:
            case WebGLTexelFormat::RGB565:
            case WebGLTexelFormat::RGB11F11F10F:
            case WebGLTexelFormat::RGBA4444:
            case WebGLTexelFormat::RGBA5551:
                return 1;
            case WebGLTexelFormat::RA8:
            case WebGLTexelFormat::RA16F:
            case WebGLTexelFormat::RA32F:
            case WebGLTexelFormat::RG8:
            case WebGLTexelFormat::RG16F:
            case WebGLTexelFormat::RG32F:
                return 2;
            case WebGLTexelFormat::RGB8:
            case WebGLTexelFormat::RGB16F:
            case WebGLTexelFormat::RGB32F:
                return 3;
            case WebGLTexelFormat::RGBA8:
            case WebGLTexelFormat::RGBA16F:
            case WebGLTexelFormat::RGBA32F:
            case WebGLTexelFormat::BGRX8:
            case WebGLTexelFormat::BGRA8:
                return 4;
            default:
                MOZ_ASSERT(false, "Unknown texel format. Coding mistake?");
                return 0;
        }
    }

    /*
     * This is the completely format-specific templatized conversion function,
     * that will be instantiated hundreds of times for all different combinations.
     * It is important to avoid generating useless code here. In particular, many
     * instantiations of this function template will never be called, so we try
     * to return immediately in these cases to allow the compiler to avoid generating
     * useless code.
     */
    template<WebGLTexelFormat SrcFormat,
             WebGLTexelFormat DstFormat,
             WebGLTexelPremultiplicationOp PremultiplicationOp>
    void run()
    {

        if (SrcFormat == DstFormat &&
            PremultiplicationOp == WebGLTexelPremultiplicationOp::None)
        {
            return;
        }


        if (!HasAlpha(SrcFormat) ||
            !HasColor(SrcFormat) ||
            !HasColor(DstFormat))
        {

            if (PremultiplicationOp != WebGLTexelPremultiplicationOp::None)
            {
                return;
            }
        }


        MOZ_ASSERT(!mAlreadyRun, "converter should be run only once!");
        mAlreadyRun = true;


        typedef
            typename DataTypeForFormat<SrcFormat>::Type
            SrcType;
        typedef
            typename DataTypeForFormat<DstFormat>::Type
            DstType;

        const WebGLTexelFormat IntermediateSrcFormat
            = IntermediateFormat<SrcFormat>::Value;
        const WebGLTexelFormat IntermediateDstFormat
            = IntermediateFormat<DstFormat>::Value;
        typedef
            typename DataTypeForFormat<IntermediateSrcFormat>::Type
            IntermediateSrcType;
        typedef
            typename DataTypeForFormat<IntermediateDstFormat>::Type
            IntermediateDstType;

        const size_t NumElementsPerSrcTexel = NumElementsPerTexelForFormat<SrcFormat>();
        const size_t NumElementsPerDstTexel = NumElementsPerTexelForFormat<DstFormat>();
        const size_t MaxElementsPerTexel = 4;
        MOZ_ASSERT(NumElementsPerSrcTexel <= MaxElementsPerTexel, "unhandled format");
        MOZ_ASSERT(NumElementsPerDstTexel <= MaxElementsPerTexel, "unhandled format");

        MOZ_ASSERT(mSrcStride % sizeof(SrcType) == 0 &&
                   mDstStride % sizeof(DstType) == 0,
                   "Unsupported: texture stride is not a multiple of sizeof(type)");
        const ptrdiff_t srcStrideInElements = mSrcStride / sizeof(SrcType);
        const ptrdiff_t dstStrideInElements = mDstStride / sizeof(DstType);

        const SrcType* srcRowStart = static_cast<const SrcType*>(mSrcStart);
        DstType* dstRowStart = static_cast<DstType*>(mDstStart);

        for (size_t i = 0; i < mHeight; ++i) {
            const SrcType* srcRowEnd = srcRowStart + mWidth * NumElementsPerSrcTexel;
            const SrcType* srcPtr = srcRowStart;
            DstType* dstPtr = dstRowStart;
            while (srcPtr != srcRowEnd) {
                IntermediateSrcType unpackedSrc[MaxElementsPerTexel];
                IntermediateDstType unpackedDst[MaxElementsPerTexel];

                unpack<SrcFormat>(srcPtr, unpackedSrc);
                convertType(unpackedSrc, unpackedDst);

                srcPtr += NumElementsPerSrcTexel;
                dstPtr += NumElementsPerDstTexel;
            }
            srcRowStart += srcStrideInElements;
            dstRowStart += dstStrideInElements;
        }

        mSuccess = true;
    }

    template<WebGLTexelFormat SrcFormat,
             WebGLTexelFormat DstFormat>
    void run(WebGLTexelPremultiplicationOp premultiplicationOp)
    {
        #define WEBGLIMAGECONVERTER_CASE_PREMULTIPLICATIONOP(PremultiplicationOp) \
            case PremultiplicationOp: \
                return run<SrcFormat, DstFormat, PremultiplicationOp>();

        switch (premultiplicationOp) {
            WEBGLIMAGECONVERTER_CASE_PREMULTIPLICATIONOP(WebGLTexelPremultiplicationOp::None)
            WEBGLIMAGECONVERTER_CASE_PREMULTIPLICATIONOP(WebGLTexelPremultiplicationOp::Premultiply)
            WEBGLIMAGECONVERTER_CASE_PREMULTIPLICATIONOP(WebGLTexelPremultiplicationOp::Unpremultiply)
            default:
                MOZ_ASSERT(false, "unhandled case. Coding mistake?");
        }

        #undef WEBGLIMAGECONVERTER_CASE_PREMULTIPLICATIONOP
    }

    template<WebGLTexelFormat SrcFormat>
    void run(WebGLTexelFormat dstFormat,
             WebGLTexelPremultiplicationOp premultiplicationOp)
    {
        #define WEBGLIMAGECONVERTER_CASE_DSTFORMAT(DstFormat) \
            case DstFormat: \
                return run<SrcFormat, DstFormat>(premultiplicationOp);

        switch (dstFormat) {
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::A8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::A16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::A32F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::R8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::R16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::R32F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RA8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RA16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RA32F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RG8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RG16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RG32F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGB565)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGB8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGB11F11F10F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGB16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGB32F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGBA4444)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGBA5551)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGBA8)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGBA16F)
        WEBGLIMAGECONVERTER_CASE_DSTFORMAT(WebGLTexelFormat::RGBA32F)

        default:
            MOZ_ASSERT(false, "unhandled case. Coding mistake?");
        }

        #undef WEBGLIMAGECONVERTER_CASE_DSTFORMAT
    }

public:

    void run(WebGLTexelFormat srcFormat,
             WebGLTexelFormat dstFormat,
             WebGLTexelPremultiplicationOp premultiplicationOp)
    {
        #define WEBGLIMAGECONVERTER_CASE_SRCFORMAT(SrcFormat) \
            case SrcFormat: \
                return run<SrcFormat>(dstFormat, premultiplicationOp);

        switch (srcFormat) {
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::A8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::A16F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::A32F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::R8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::R16F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::R32F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RA8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RA16F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RA32F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGB565)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGB8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGB16F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGB32F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGBA4444)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGBA5551)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGBA8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGBA16F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::RGBA32F)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::BGRX8)
        WEBGLIMAGECONVERTER_CASE_SRCFORMAT(WebGLTexelFormat::BGRA8)

        default:
            MOZ_ASSERT(false, "unhandled case. Coding mistake?");
        }

        #undef WEBGLIMAGECONVERTER_CASE_SRCFORMAT
    }

    WebGLImageConverter(size_t width, size_t height,
                        const void* srcStart, void* dstStart,
                        ptrdiff_t srcStride, ptrdiff_t dstStride)
        : mWidth(width), mHeight(height),
          mSrcStart(srcStart), mDstStart(dstStart),
          mSrcStride(srcStride), mDstStride(dstStride),
          mAlreadyRun(false), mSuccess(false)
    {}

    bool Success() const {
        return mSuccess;
    }
};

} // end anonymous namespace

bool
ConvertImage(size_t width, size_t height,
             const void* srcBegin, size_t srcStride, gl::OriginPos srcOrigin,
             WebGLTexelFormat srcFormat, bool srcPremultiplied,
             void* dstBegin, size_t dstStride, gl::OriginPos dstOrigin,
             WebGLTexelFormat dstFormat, bool dstPremultiplied,
             bool* const out_wasTrivial)
{
    *out_wasTrivial = true;

    if (srcFormat == WebGLTexelFormat::FormatNotSupportingAnyConversion ||
        dstFormat == WebGLTexelFormat::FormatNotSupportingAnyConversion)
    {
        return false;
    }

    if (!width || !height)
        return true;

    const bool shouldYFlip = (srcOrigin != dstOrigin);

    const bool canSkipPremult = (!HasAlpha(srcFormat) ||
                                 !HasColor(srcFormat) ||
                                 !HasColor(dstFormat));

    WebGLTexelPremultiplicationOp premultOp;
    if (canSkipPremult) {
        premultOp = WebGLTexelPremultiplicationOp::None;
    } else if (!srcPremultiplied && dstPremultiplied) {
        premultOp = WebGLTexelPremultiplicationOp::Premultiply;
    } else if (srcPremultiplied && !dstPremultiplied) {
        premultOp = WebGLTexelPremultiplicationOp::Unpremultiply;
    } else {
        premultOp = WebGLTexelPremultiplicationOp::None;
    }

    const uint8_t* srcItr = (const uint8_t*)srcBegin;
    const uint8_t* const srcEnd = srcItr + srcStride * height;
    uint8_t* dstItr = (uint8_t*)dstBegin;
    ptrdiff_t dstItrStride = dstStride;
    if (shouldYFlip) {
         dstItr = dstItr + dstStride * (height - 1);
         dstItrStride = -dstItrStride;
    }

    if (srcFormat == dstFormat && premultOp == WebGLTexelPremultiplicationOp::None) {

        MOZ_ASSERT((srcPremultiplied != dstPremultiplied ||
                    shouldYFlip ||
                    srcStride != dstStride),
                   "Performance trap -- should handle this case earlier to avoid memcpy");

        const auto bytesPerPixel = TexelBytesForFormat(srcFormat);
        const size_t bytesPerRow = bytesPerPixel * width;

        while (srcItr != srcEnd) {
            memcpy(dstItr, srcItr, bytesPerRow);
            srcItr += srcStride;
            dstItr += dstItrStride;
        }
        return true;
    }

    *out_wasTrivial = false;

    WebGLImageConverter converter(width, height, srcItr, dstItr, srcStride, dstItrStride);
    converter.run(srcFormat, dstFormat, premultOp);

    if (!converter.Success()) {
        MOZ_CRASH("programming mistake in WebGL texture conversions");
    }

    return true;
}

} // end namespace mozilla

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebGLFormats.h"

#include "GLConsts.h"

#ifdef FOO
#error FOO is already defined! We use FOO() macros to keep things succinct in this file.
#endif

namespace mozilla {
namespace webgl {

template<typename K, typename V, typename K2, typename V2>
static inline void
AlwaysInsert(std::map<K,V>& dest, const K2& key, const V2& val)
{
    auto res = dest.insert({ key, val });
    bool didInsert = res.second;
    MOZ_ALWAYS_TRUE(didInsert);
}

template<typename K, typename V, typename K2>
static inline V*
FindOrNull(const std::map<K,V*>& dest, const K2& key)
{
    auto itr = dest.find(key);
    if (itr == dest.end())
        return nullptr;

    return itr->second;
}

template<typename K, typename V, typename K2>
static inline V*
FindPtrOrNull(std::map<K,V>& dest, const K2& key)
{
    auto itr = dest.find(key);
    if (itr == dest.end())
        return nullptr;

    return &(itr->second);
}


std::map<EffectiveFormat, const CompressedFormatInfo> gCompressedFormatInfoMap;
std::map<EffectiveFormat, FormatInfo> gFormatInfoMap;

static inline const CompressedFormatInfo*
GetCompressedFormatInfo(EffectiveFormat format)
{
    MOZ_ASSERT(!gCompressedFormatInfoMap.empty());
    return FindPtrOrNull(gCompressedFormatInfoMap, format);
}

static inline FormatInfo*
GetFormatInfo_NoLock(EffectiveFormat format)
{
    MOZ_ASSERT(!gFormatInfoMap.empty());
    return FindPtrOrNull(gFormatInfoMap, format);
}


static void
AddCompressedFormatInfo(EffectiveFormat format, uint16_t bitsPerBlock, uint8_t blockWidth,
                        uint8_t blockHeight, CompressionFamily family)
{
    MOZ_ASSERT(bitsPerBlock % 8 == 0);
    uint16_t bytesPerBlock = bitsPerBlock / 8; // The specs always state these in bits,
    MOZ_ASSERT(bytesPerBlock <= 255);

    const CompressedFormatInfo info = { format, uint8_t(bytesPerBlock), blockWidth,
                                        blockHeight, family };
    AlwaysInsert(gCompressedFormatInfoMap, format, info);
}

static void
InitCompressedFormatInfo()
{
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGB8_ETC2                     ,  64, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ETC2                    ,  64, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA8_ETC2_EAC                , 128, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC         , 128, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_R11_EAC                       ,  64, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RG11_EAC                      , 128, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SIGNED_R11_EAC                ,  64, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SIGNED_RG11_EAC               , 128, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 ,  64, 4, 4, CompressionFamily::ES3);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  64, 4, 4, CompressionFamily::ES3);

    AddCompressedFormatInfo(EffectiveFormat::ATC_RGB_AMD                    ,  64, 4, 4, CompressionFamily::ATC);
    AddCompressedFormatInfo(EffectiveFormat::ATC_RGBA_EXPLICIT_ALPHA_AMD    , 128, 4, 4, CompressionFamily::ATC);
    AddCompressedFormatInfo(EffectiveFormat::ATC_RGBA_INTERPOLATED_ALPHA_AMD, 128, 4, 4, CompressionFamily::ATC);

    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGB_S3TC_DXT1_EXT ,  64, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT,  64, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT, 128, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT, 128, 4, 4, CompressionFamily::S3TC);

    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT ,       64, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  64, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 128, 4, 4, CompressionFamily::S3TC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 128, 4, 4, CompressionFamily::S3TC);

    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_4x4_KHR          , 128,  4,  4, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_5x4_KHR          , 128,  5,  4, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_5x5_KHR          , 128,  5,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_6x5_KHR          , 128,  6,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_6x6_KHR          , 128,  6,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_8x5_KHR          , 128,  8,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_8x6_KHR          , 128,  8,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_8x8_KHR          , 128,  8,  8, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_10x5_KHR         , 128, 10,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_10x6_KHR         , 128, 10,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_10x8_KHR         , 128, 10,  8, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_10x10_KHR        , 128, 10, 10, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_12x10_KHR        , 128, 12, 10, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_ASTC_12x12_KHR        , 128, 12, 12, CompressionFamily::ASTC);

    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR  , 128,  4,  4, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR  , 128,  5,  4, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR  , 128,  5,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR  , 128,  6,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR  , 128,  6,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR  , 128,  8,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR  , 128,  8,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR  , 128,  8,  8, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR , 128, 10,  5, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR , 128, 10,  6, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR , 128, 10,  8, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 128, 10, 10, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 128, 12, 10, CompressionFamily::ASTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 128, 12, 12, CompressionFamily::ASTC);

    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGB_PVRTC_4BPPV1 , 256,  8, 8, CompressionFamily::PVRTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_PVRTC_4BPPV1, 256,  8, 8, CompressionFamily::PVRTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGB_PVRTC_2BPPV1 , 256, 16, 8, CompressionFamily::PVRTC);
    AddCompressedFormatInfo(EffectiveFormat::COMPRESSED_RGBA_PVRTC_2BPPV1, 256, 16, 8, CompressionFamily::PVRTC);

    AddCompressedFormatInfo(EffectiveFormat::ETC1_RGB8_OES, 64, 4, 4, CompressionFamily::ETC1);
}


static void
AddFormatInfo(EffectiveFormat format, const char* name, GLenum sizedFormat,
              uint8_t bytesPerPixel, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
              uint8_t d, uint8_t s, UnsizedFormat unsizedFormat, bool isSRGB,
              ComponentType componentType)
{
    switch (unsizedFormat) {
    case UnsizedFormat::R:
        MOZ_ASSERT(r && !g && !b && !a && !d && !s);
        break;

    case UnsizedFormat::RG:
        MOZ_ASSERT(r && g && !b && !a && !d && !s);
        break;

    case UnsizedFormat::RGB:
        MOZ_ASSERT(r && g && b && !a && !d && !s);
        break;

    case UnsizedFormat::RGBA:
        MOZ_ASSERT(r && g && b && a && !d && !s);
        break;

    case UnsizedFormat::L:
        MOZ_ASSERT(r && !g && !b && !a && !d && !s);
        break;

    case UnsizedFormat::A:
        MOZ_ASSERT(!r && !g && !b && a && !d && !s);
        break;

    case UnsizedFormat::LA:
        MOZ_ASSERT(r && !g && !b && a && !d && !s);
        break;

    case UnsizedFormat::D:
        MOZ_ASSERT(!r && !g && !b && !a && d && !s);
        break;

    case UnsizedFormat::S:
        MOZ_ASSERT(!r && !g && !b && !a && !d && s);
        break;

    case UnsizedFormat::DEPTH_STENCIL:
        MOZ_ASSERT(!r && !g && !b && !a && d && s);
        break;
    }

    const CompressedFormatInfo* compressedFormatInfo = GetCompressedFormatInfo(format);
    MOZ_ASSERT(!bytesPerPixel == bool(compressedFormatInfo));

#ifdef DEBUG
    uint8_t totalBits = r + g + b + a + d + s;
    if (format == EffectiveFormat::RGB9_E5) {
        totalBits = 9 + 9 + 9 + 5;
    }

    if (compressedFormatInfo) {
        MOZ_ASSERT(totalBits);
        MOZ_ASSERT(!bytesPerPixel);
    } else {
        MOZ_ASSERT(totalBits == bytesPerPixel*8);
    }
#endif

    const FormatInfo info = { format, name, sizedFormat, unsizedFormat, componentType,
                              isSRGB, compressedFormatInfo, bytesPerPixel, r,g,b,a,d,s };
    AlwaysInsert(gFormatInfoMap, format, info);
}

static void
InitFormatInfo()
{
#define FOO(x) EffectiveFormat::x, #x, LOCAL_GL_ ## x
    AddFormatInfo(FOO(R8            ),  1,  8, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(R8_SNORM      ),  1,  8, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::NormInt );
    AddFormatInfo(FOO(RG8           ),  2,  8, 8, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RG8_SNORM     ),  2,  8, 8, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::NormInt );
    AddFormatInfo(FOO(RGB8          ),  3,  8, 8, 8, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGB8_SNORM    ),  3,  8, 8, 8, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::NormInt );
    AddFormatInfo(FOO(RGB565        ),  2,  5, 6, 5, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGBA4         ),  2,  4, 4, 4, 4,  0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGB5_A1       ),  2,  5, 5, 5, 1,  0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGBA8         ),  4,  8, 8, 8, 8,  0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGBA8_SNORM   ),  4,  8, 8, 8, 8,  0,0, UnsizedFormat::RGBA, false, ComponentType::NormInt );
    AddFormatInfo(FOO(RGB10_A2      ),  4, 10,10,10, 2,  0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(RGB10_A2UI    ),  4, 10,10,10, 2,  0,0, UnsizedFormat::RGBA, false, ComponentType::UInt    );

    AddFormatInfo(FOO(SRGB8         ),  3,  8, 8, 8, 0,  0,0, UnsizedFormat::RGB , true , ComponentType::NormUInt);
    AddFormatInfo(FOO(SRGB8_ALPHA8  ),  4,  8, 8, 8, 8,  0,0, UnsizedFormat::RGBA, true , ComponentType::NormUInt);

    AddFormatInfo(FOO(R16F          ),  2, 16, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::Float   );
    AddFormatInfo(FOO(RG16F         ),  4, 16,16, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::Float   );
    AddFormatInfo(FOO(RGB16F        ),  6, 16,16,16, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Float   );
    AddFormatInfo(FOO(RGBA16F       ),  8, 16,16,16,16,  0,0, UnsizedFormat::RGBA, false, ComponentType::Float   );
    AddFormatInfo(FOO(R32F          ),  4, 32, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::Float   );
    AddFormatInfo(FOO(RG32F         ),  8, 32,32, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::Float   );
    AddFormatInfo(FOO(RGB32F        ), 12, 32,32,32, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Float   );
    AddFormatInfo(FOO(RGBA32F       ), 16, 32,32,32,32,  0,0, UnsizedFormat::RGBA, false, ComponentType::Float   );

    AddFormatInfo(FOO(R11F_G11F_B10F),  4, 11,11,10, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Float   );
    AddFormatInfo(FOO(RGB9_E5       ),  4, 14,14,14, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Float   );

    AddFormatInfo(FOO(R8I           ),  1,  8, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::Int     );
    AddFormatInfo(FOO(R8UI          ),  1,  8, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::UInt    );
    AddFormatInfo(FOO(R16I          ),  2, 16, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::Int     );
    AddFormatInfo(FOO(R16UI         ),  2, 16, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::UInt    );
    AddFormatInfo(FOO(R32I          ),  4, 32, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::Int     );
    AddFormatInfo(FOO(R32UI         ),  4, 32, 0, 0, 0,  0,0, UnsizedFormat::R   , false, ComponentType::UInt    );

    AddFormatInfo(FOO(RG8I          ),  2,  8, 8, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::Int     );
    AddFormatInfo(FOO(RG8UI         ),  2,  8, 8, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::UInt    );
    AddFormatInfo(FOO(RG16I         ),  4, 16,16, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::Int     );
    AddFormatInfo(FOO(RG16UI        ),  4, 16,16, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::UInt    );
    AddFormatInfo(FOO(RG32I         ),  8, 32,32, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::Int     );
    AddFormatInfo(FOO(RG32UI        ),  8, 32,32, 0, 0,  0,0, UnsizedFormat::RG  , false, ComponentType::UInt    );

    AddFormatInfo(FOO(RGB8I         ),  3,  8, 8, 8, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Int     );
    AddFormatInfo(FOO(RGB8UI        ),  3,  8, 8, 8, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::UInt    );
    AddFormatInfo(FOO(RGB16I        ),  6, 16,16,16, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Int     );
    AddFormatInfo(FOO(RGB16UI       ),  6, 16,16,16, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::UInt    );
    AddFormatInfo(FOO(RGB32I        ), 12, 32,32,32, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::Int     );
    AddFormatInfo(FOO(RGB32UI       ), 12, 32,32,32, 0,  0,0, UnsizedFormat::RGB , false, ComponentType::UInt    );

    AddFormatInfo(FOO(RGBA8I        ),  4,  8, 8, 8, 8,  0,0, UnsizedFormat::RGBA, false, ComponentType::Int     );
    AddFormatInfo(FOO(RGBA8UI       ),  4,  8, 8, 8, 8,  0,0, UnsizedFormat::RGBA, false, ComponentType::UInt    );
    AddFormatInfo(FOO(RGBA16I       ),  8, 16,16,16,16,  0,0, UnsizedFormat::RGBA, false, ComponentType::Int     );
    AddFormatInfo(FOO(RGBA16UI      ),  8, 16,16,16,16,  0,0, UnsizedFormat::RGBA, false, ComponentType::UInt    );
    AddFormatInfo(FOO(RGBA32I       ), 16, 32,32,32,32,  0,0, UnsizedFormat::RGBA, false, ComponentType::Int     );
    AddFormatInfo(FOO(RGBA32UI      ), 16, 32,32,32,32,  0,0, UnsizedFormat::RGBA, false, ComponentType::UInt    );

    AddFormatInfo(FOO(DEPTH_COMPONENT16 ), 2, 0,0,0,0, 16,0, UnsizedFormat::D , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(DEPTH_COMPONENT24 ), 3, 0,0,0,0, 24,0, UnsizedFormat::D , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(DEPTH_COMPONENT32F), 4, 0,0,0,0, 32,0, UnsizedFormat::D , false, ComponentType::Float);
    AddFormatInfo(FOO(DEPTH24_STENCIL8  ), 4, 0,0,0,0, 24,8, UnsizedFormat::DEPTH_STENCIL, false, ComponentType::Special);
    AddFormatInfo(FOO(DEPTH32F_STENCIL8 ), 5, 0,0,0,0, 32,8, UnsizedFormat::DEPTH_STENCIL, false, ComponentType::Special);

    AddFormatInfo(FOO(STENCIL_INDEX8), 1, 0,0,0,0, 0,8, UnsizedFormat::S, false, ComponentType::UInt);



    AddFormatInfo(FOO(COMPRESSED_RGB_S3TC_DXT1_EXT ), 0, 1,1,1,0, 0,0, UnsizedFormat::RGB , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_RGBA_S3TC_DXT1_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_RGBA_S3TC_DXT3_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_RGBA_S3TC_DXT5_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, false, ComponentType::NormUInt);

    AddFormatInfo(FOO(COMPRESSED_SRGB_S3TC_DXT1_EXT ), 0, 1,1,1,0, 0,0, UnsizedFormat::RGB , true, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, true, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, true, ComponentType::NormUInt);
    AddFormatInfo(FOO(COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT), 0, 1,1,1,1, 0,0, UnsizedFormat::RGBA, true, ComponentType::NormUInt);


#undef FOO

#define FOO(x) EffectiveFormat::x, #x, 0

    AddFormatInfo(FOO(Luminance8Alpha8), 2, 8,0,0,8, 0,0, UnsizedFormat::LA, false, ComponentType::NormUInt);
    AddFormatInfo(FOO(Luminance8      ), 1, 8,0,0,0, 0,0, UnsizedFormat::L , false, ComponentType::NormUInt);
    AddFormatInfo(FOO(Alpha8          ), 1, 0,0,0,8, 0,0, UnsizedFormat::A , false, ComponentType::NormUInt);

    AddFormatInfo(FOO(Luminance32FAlpha32F), 8, 32,0,0,32, 0,0, UnsizedFormat::LA, false, ComponentType::Float);
    AddFormatInfo(FOO(Luminance32F        ), 4, 32,0,0, 0, 0,0, UnsizedFormat::L , false, ComponentType::Float);
    AddFormatInfo(FOO(Alpha32F            ), 4,  0,0,0,32, 0,0, UnsizedFormat::A , false, ComponentType::Float);

    AddFormatInfo(FOO(Luminance16FAlpha16F), 4, 16,0,0,16, 0,0, UnsizedFormat::LA, false, ComponentType::Float);
    AddFormatInfo(FOO(Luminance16F        ), 2, 16,0,0, 0, 0,0, UnsizedFormat::L , false, ComponentType::Float);
    AddFormatInfo(FOO(Alpha16F            ), 2,  0,0,0,16, 0,0, UnsizedFormat::A , false, ComponentType::Float);

#undef FOO


    const auto fnSetCopyDecay = [](EffectiveFormat src, EffectiveFormat asR,
                                   EffectiveFormat asRG, EffectiveFormat asRGB,
                                   EffectiveFormat asRGBA, EffectiveFormat asL,
                                   EffectiveFormat asA, EffectiveFormat asLA)
    {
        auto& map = GetFormatInfo_NoLock(src)->copyDecayFormats;

        const auto fnSet = [&map](UnsizedFormat uf, EffectiveFormat ef) {
            if (ef == EffectiveFormat::MAX)
                return;

            const auto* format = GetFormatInfo_NoLock(ef);
            MOZ_ASSERT(format->unsizedFormat == uf);
            AlwaysInsert(map, uf, format);
        };

        fnSet(UnsizedFormat::R   , asR);
        fnSet(UnsizedFormat::RG  , asRG);
        fnSet(UnsizedFormat::RGB , asRGB);
        fnSet(UnsizedFormat::RGBA, asRGBA);
        fnSet(UnsizedFormat::L   , asL);
        fnSet(UnsizedFormat::A   , asA);
        fnSet(UnsizedFormat::LA  , asLA);
    };

#define SET_COPY_DECAY(src,asR,asRG,asRGB,asRGBA,asL,asA,asLA) \
    fnSetCopyDecay(EffectiveFormat::src, EffectiveFormat::asR, EffectiveFormat::asRG,     \
                   EffectiveFormat::asRGB, EffectiveFormat::asRGBA, EffectiveFormat::asL, \
                   EffectiveFormat::asA, EffectiveFormat::asLA);


#define SET_BY_SUFFIX(X) \
        SET_COPY_DECAY(   R##X, R##X,   MAX,    MAX,     MAX, Luminance##X,      MAX,                    MAX) \
        SET_COPY_DECAY(  RG##X, R##X, RG##X,    MAX,     MAX, Luminance##X,      MAX,                    MAX) \
        SET_COPY_DECAY( RGB##X, R##X, RG##X, RGB##X,     MAX, Luminance##X,      MAX,                    MAX) \
        SET_COPY_DECAY(RGBA##X, R##X, RG##X, RGB##X, RGBA##X, Luminance##X, Alpha##X, Luminance##X##Alpha##X)

    SET_BY_SUFFIX(8)   // WebGL decided that RGB8 should be guaranteed renderable.
    SET_BY_SUFFIX(16F) // RGB16F is renderable in EXT_color_buffer_half_float, though not
    SET_BY_SUFFIX(32F) // Technically RGB32F is never renderable, but no harm here.

#undef SET_BY_SUFFIX


#define SET_BY_SUFFIX(X) \
        SET_COPY_DECAY(   R##X, R##X,   MAX,    MAX,     MAX, MAX, MAX, MAX) \
        SET_COPY_DECAY(  RG##X, R##X, RG##X,    MAX,     MAX, MAX, MAX, MAX) \
        SET_COPY_DECAY(RGBA##X, R##X, RG##X, RGB##X, RGBA##X, MAX, MAX, MAX)

    SET_BY_SUFFIX(8I)
    SET_BY_SUFFIX(8UI)

    SET_BY_SUFFIX(16I)
    SET_BY_SUFFIX(16UI)

    SET_BY_SUFFIX(32I)
    SET_BY_SUFFIX(32UI)

#undef SET_BY_SUFFIX


    SET_COPY_DECAY(    RGB565, R8, RG8, RGB565,      MAX, Luminance8,    MAX,              MAX)
    SET_COPY_DECAY(     RGBA4, R8, RG8, RGB565,    RGBA4, Luminance8, Alpha8, Luminance8Alpha8)
    SET_COPY_DECAY(   RGB5_A1, R8, RG8, RGB565,  RGB5_A1, Luminance8, Alpha8, Luminance8Alpha8)
    SET_COPY_DECAY(  RGB10_A2, R8, RG8,   RGB8, RGB10_A2, Luminance8, Alpha8,              MAX)

    SET_COPY_DECAY(RGB10_A2UI, R8UI, RG8UI, RGB8UI, RGB10_A2UI, MAX, MAX, MAX)

    SET_COPY_DECAY(SRGB8_ALPHA8, MAX, MAX, MAX, SRGB8_ALPHA8, MAX, Alpha8, MAX)

    SET_COPY_DECAY(R11F_G11F_B10F, R16F, RG16F, R11F_G11F_B10F, MAX, Luminance16F, MAX, MAX)

#undef SET_COPY_DECAY
}


bool gAreFormatTablesInitialized = false;

static void
EnsureInitFormatTables(/* cjh const StaticMutexAutoLock&*/) // Prove that you locked it!
{
    if (MOZ_LIKELY(gAreFormatTablesInitialized))
        return;

    gAreFormatTablesInitialized = true;

    InitCompressedFormatInfo();
    InitFormatInfo();
}



const FormatInfo*
GetFormat(EffectiveFormat format)
{
    EnsureInitFormatTables();

    return GetFormatInfo_NoLock(format);
}


const FormatInfo*
FormatInfo::GetCopyDecayFormat(UnsizedFormat uf) const
{
    return FindOrNull(this->copyDecayFormats, uf);
}

bool
GetBytesPerPixel(const PackingInfo& packing, uint8_t* const out_bytes)
{
    uint8_t bytesPerChannel;

    switch (packing.type) {
    case LOCAL_GL_UNSIGNED_SHORT_4_4_4_4:
    case LOCAL_GL_UNSIGNED_SHORT_5_5_5_1:
    case LOCAL_GL_UNSIGNED_SHORT_5_6_5:
        *out_bytes = 2;
        return true;

    case LOCAL_GL_UNSIGNED_INT_10F_11F_11F_REV:
    case LOCAL_GL_UNSIGNED_INT_2_10_10_10_REV:
    case LOCAL_GL_UNSIGNED_INT_24_8:
    case LOCAL_GL_UNSIGNED_INT_5_9_9_9_REV:
        *out_bytes = 4;
        return true;

    case LOCAL_GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        *out_bytes = 8;
        return true;


    case LOCAL_GL_BYTE:
    case LOCAL_GL_UNSIGNED_BYTE:
        bytesPerChannel = 1;
        break;

    case LOCAL_GL_SHORT:
    case LOCAL_GL_UNSIGNED_SHORT:
    case LOCAL_GL_HALF_FLOAT:
    case LOCAL_GL_HALF_FLOAT_OES:
        bytesPerChannel = 2;
        break;

    case LOCAL_GL_INT:
    case LOCAL_GL_UNSIGNED_INT:
    case LOCAL_GL_FLOAT:
        bytesPerChannel = 4;
        break;

    default:
        return false;
    }

    uint8_t channels;

    switch (packing.format) {
    case LOCAL_GL_RED:
    case LOCAL_GL_RED_INTEGER:
    case LOCAL_GL_LUMINANCE:
    case LOCAL_GL_ALPHA:
    case LOCAL_GL_DEPTH_COMPONENT:
        channels = 1;
        break;

    case LOCAL_GL_RG:
    case LOCAL_GL_RG_INTEGER:
    case LOCAL_GL_LUMINANCE_ALPHA:
    case LOCAL_GL_DEPTH_COMPONENT16:
        channels = 2;
        break;

    case LOCAL_GL_RGB:
    case LOCAL_GL_RGB_INTEGER:
    case LOCAL_GL_SRGB:
    case LOCAL_GL_DEPTH_COMPONENT24:
        channels = 3;
        break;

    case LOCAL_GL_BGRA:
    case LOCAL_GL_RGBA:
    case LOCAL_GL_RGBA_INTEGER:
    case LOCAL_GL_SRGB_ALPHA:
    case LOCAL_GL_DEPTH_COMPONENT32F:
        channels = 4;
        break;

    default:
        return false;
    }

    *out_bytes = bytesPerChannel * channels;
    return true;
}

uint8_t
BytesPerPixel(const PackingInfo& packing)
{
    uint8_t ret;
    if (MOZ_LIKELY(GetBytesPerPixel(packing, &ret)))
        return ret;

    MOZ_CRASH("Bad `packing`.");
}


bool
FormatUsageInfo::IsUnpackValid(const PackingInfo& key,
                               const DriverUnpackInfo** const out_value) const
{
    auto itr = validUnpacks.find(key);
    if (itr == validUnpacks.end())
        return false;

    *out_value = &(itr->second);
    return true;
}

void
FormatUsageInfo::ResolveMaxSamples(gl::GLContext* gl)
{
}


static void
AddSimpleUnsized(FormatUsageAuthority* fua, GLenum unpackFormat, GLenum unpackType,
                 EffectiveFormat effFormat)
{
    auto usage = fua->EditUsage(effFormat);
    usage->isFilterable = true;

    const PackingInfo pi = {unpackFormat, unpackType};
    const DriverUnpackInfo dui = {unpackFormat, unpackFormat, unpackType};
    fua->AddTexUnpack(usage, pi, dui);

    fua->AllowUnsizedTexFormat(pi, usage);
};


/*static*/ const GLint FormatUsageInfo::kLuminanceSwizzleRGBA[4] = { LOCAL_GL_RED,
                                                                     LOCAL_GL_RED,
                                                                     LOCAL_GL_RED,
                                                                     LOCAL_GL_ONE };
/*static*/ const GLint FormatUsageInfo::kAlphaSwizzleRGBA[4] = { LOCAL_GL_ZERO,
                                                                 LOCAL_GL_ZERO,
                                                                 LOCAL_GL_ZERO,
                                                                 LOCAL_GL_RED };
/*static*/ const GLint FormatUsageInfo::kLumAlphaSwizzleRGBA[4] = { LOCAL_GL_RED,
                                                                    LOCAL_GL_RED,
                                                                    LOCAL_GL_RED,
                                                                    LOCAL_GL_GREEN };

static bool
AddLegacyFormats_LA8(FormatUsageAuthority* fua, gl::GLContext* gl)
{
    {
        AddSimpleUnsized(fua, LOCAL_GL_LUMINANCE      , LOCAL_GL_UNSIGNED_BYTE, EffectiveFormat::Luminance8      );
        AddSimpleUnsized(fua, LOCAL_GL_ALPHA          , LOCAL_GL_UNSIGNED_BYTE, EffectiveFormat::Alpha8          );
        AddSimpleUnsized(fua, LOCAL_GL_LUMINANCE_ALPHA, LOCAL_GL_UNSIGNED_BYTE, EffectiveFormat::Luminance8Alpha8);
    }

    return true;
}

static bool
AddUnsizedFormats(FormatUsageAuthority* fua, gl::GLContext* gl)
{
    AddSimpleUnsized(fua, LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_BYTE         , EffectiveFormat::RGBA8  );
    AddSimpleUnsized(fua, LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_SHORT_4_4_4_4, EffectiveFormat::RGBA4  );
    AddSimpleUnsized(fua, LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_SHORT_5_5_5_1, EffectiveFormat::RGB5_A1);
    AddSimpleUnsized(fua, LOCAL_GL_RGB , LOCAL_GL_UNSIGNED_BYTE         , EffectiveFormat::RGB8   );
    AddSimpleUnsized(fua, LOCAL_GL_RGB , LOCAL_GL_UNSIGNED_SHORT_5_6_5  , EffectiveFormat::RGB565 );

    return AddLegacyFormats_LA8(fua, gl);
}

void
FormatUsageInfo::SetRenderable()
{
    this->isRenderable = true;

#ifdef DEBUG
    const auto format = this->format;
    if (format->IsColorFormat()) {
        const auto& map = format->copyDecayFormats;
        const auto itr = map.find(format->unsizedFormat);
        MOZ_ASSERT(itr != map.end(), "Renderable formats must be in copyDecayFormats.");
        MOZ_ASSERT(itr->second == format);
    }
#endif
}

UniquePtr<FormatUsageAuthority>
FormatUsageAuthority::CreateForWebGL1(gl::GLContext* gl)
{
    UniquePtr<FormatUsageAuthority> ret(new FormatUsageAuthority);
    const auto ptr = ret.get();


    const auto fnSet = [ptr](EffectiveFormat effFormat, bool isRenderable,
                             bool isFilterable)
    {
        MOZ_ASSERT(!ptr->GetUsage(effFormat));

        auto usage = ptr->EditUsage(effFormat);
        usage->isFilterable = isFilterable;

        if (isRenderable) {
            usage->SetRenderable();
        }
    };

    fnSet(EffectiveFormat::RGBA8  , true, true);
    fnSet(EffectiveFormat::RGBA4  , true, true);
    fnSet(EffectiveFormat::RGB5_A1, true, true);
    fnSet(EffectiveFormat::RGB565 , true, true);

    fnSet(EffectiveFormat::RGB8, true, true);

    fnSet(EffectiveFormat::Luminance8Alpha8, false, true);
    fnSet(EffectiveFormat::Luminance8      , false, true);
    fnSet(EffectiveFormat::Alpha8          , false, true);

    fnSet(EffectiveFormat::DEPTH_COMPONENT16, true, false);
    fnSet(EffectiveFormat::STENCIL_INDEX8   , true, false);

    fnSet(EffectiveFormat::DEPTH24_STENCIL8, true, false);


#define FOO(x) ptr->AllowRBFormat(LOCAL_GL_ ## x, ptr->GetUsage(EffectiveFormat::x))

    FOO(RGBA4            );
    FOO(RGB5_A1          );
    FOO(RGB565           );
    FOO(DEPTH_COMPONENT16);
    FOO(STENCIL_INDEX8   );

#undef FOO

    ptr->AllowRBFormat(LOCAL_GL_DEPTH_STENCIL,
                       ptr->GetUsage(EffectiveFormat::DEPTH24_STENCIL8));


    if (!AddUnsizedFormats(ptr, gl))
        return nullptr;

    return Move(ret);
}

UniquePtr<FormatUsageAuthority>
FormatUsageAuthority::CreateForWebGL2(gl::GLContext* gl)
{
    UniquePtr<FormatUsageAuthority> ret(new FormatUsageAuthority);
    const auto ptr = ret.get();


    const auto fnAddSizedUnpack = [ptr](EffectiveFormat effFormat, GLenum internalFormat,
                                        GLenum unpackFormat, GLenum unpackType)
    {
        auto usage = ptr->EditUsage(effFormat);

        const PackingInfo pi = {unpackFormat, unpackType};
        const DriverUnpackInfo dui = {internalFormat, unpackFormat, unpackType};
        ptr->AddTexUnpack(usage, pi, dui);
    };

#define FOO(x) EffectiveFormat::x, LOCAL_GL_ ## x

    fnAddSizedUnpack(FOO(RGBA8       ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_BYTE              );
    fnAddSizedUnpack(FOO(RGBA4       ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_SHORT_4_4_4_4     );
    fnAddSizedUnpack(FOO(RGBA4       ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_BYTE              );
    fnAddSizedUnpack(FOO(RGB5_A1     ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_SHORT_5_5_5_1     );
    fnAddSizedUnpack(FOO(RGB5_A1     ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_BYTE              );
    fnAddSizedUnpack(FOO(RGB5_A1     ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_INT_2_10_10_10_REV);
    fnAddSizedUnpack(FOO(SRGB8_ALPHA8), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_BYTE              );
    fnAddSizedUnpack(FOO(RGBA8_SNORM ), LOCAL_GL_RGBA, LOCAL_GL_BYTE                       );
    fnAddSizedUnpack(FOO(RGB10_A2    ), LOCAL_GL_RGBA, LOCAL_GL_UNSIGNED_INT_2_10_10_10_REV);
    fnAddSizedUnpack(FOO(RGBA16F     ), LOCAL_GL_RGBA, LOCAL_GL_HALF_FLOAT                 );
    fnAddSizedUnpack(FOO(RGBA16F     ), LOCAL_GL_RGBA, LOCAL_GL_FLOAT                      );
    fnAddSizedUnpack(FOO(RGBA32F     ), LOCAL_GL_RGBA, LOCAL_GL_FLOAT                      );

    fnAddSizedUnpack(FOO(RGBA8UI   ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_UNSIGNED_BYTE              );
    fnAddSizedUnpack(FOO(RGBA8I    ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_BYTE                       );
    fnAddSizedUnpack(FOO(RGBA16UI  ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_UNSIGNED_SHORT             );
    fnAddSizedUnpack(FOO(RGBA16I   ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_SHORT                      );
    fnAddSizedUnpack(FOO(RGBA32UI  ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_UNSIGNED_INT               );
    fnAddSizedUnpack(FOO(RGBA32I   ), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_INT                        );
    fnAddSizedUnpack(FOO(RGB10_A2UI), LOCAL_GL_RGBA_INTEGER, LOCAL_GL_UNSIGNED_INT_2_10_10_10_REV);

    fnAddSizedUnpack(FOO(RGB8          ), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_BYTE               );
    fnAddSizedUnpack(FOO(SRGB8         ), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_BYTE               );
    fnAddSizedUnpack(FOO(RGB565        ), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_SHORT_5_6_5        );
    fnAddSizedUnpack(FOO(RGB565        ), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_BYTE               );
    fnAddSizedUnpack(FOO(RGB8_SNORM    ), LOCAL_GL_RGB, LOCAL_GL_BYTE                        );
    fnAddSizedUnpack(FOO(R11F_G11F_B10F), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_INT_10F_11F_11F_REV);
    fnAddSizedUnpack(FOO(R11F_G11F_B10F), LOCAL_GL_RGB, LOCAL_GL_HALF_FLOAT                  );
    fnAddSizedUnpack(FOO(R11F_G11F_B10F), LOCAL_GL_RGB, LOCAL_GL_FLOAT                       );
    fnAddSizedUnpack(FOO(RGB16F        ), LOCAL_GL_RGB, LOCAL_GL_HALF_FLOAT                  );
    fnAddSizedUnpack(FOO(RGB16F        ), LOCAL_GL_RGB, LOCAL_GL_FLOAT                       );
    fnAddSizedUnpack(FOO(RGB9_E5       ), LOCAL_GL_RGB, LOCAL_GL_UNSIGNED_INT_5_9_9_9_REV    );
    fnAddSizedUnpack(FOO(RGB9_E5       ), LOCAL_GL_RGB, LOCAL_GL_HALF_FLOAT                  );
    fnAddSizedUnpack(FOO(RGB9_E5       ), LOCAL_GL_RGB, LOCAL_GL_FLOAT                       );
    fnAddSizedUnpack(FOO(RGB32F        ), LOCAL_GL_RGB, LOCAL_GL_FLOAT                       );

    fnAddSizedUnpack(FOO(RGB8UI ), LOCAL_GL_RGB_INTEGER, LOCAL_GL_UNSIGNED_BYTE );
    fnAddSizedUnpack(FOO(RGB8I  ), LOCAL_GL_RGB_INTEGER, LOCAL_GL_BYTE          );
    fnAddSizedUnpack(FOO(RGB16UI), LOCAL_GL_RGB_INTEGER, LOCAL_GL_UNSIGNED_SHORT);
    fnAddSizedUnpack(FOO(RGB16I ), LOCAL_GL_RGB_INTEGER, LOCAL_GL_SHORT         );
    fnAddSizedUnpack(FOO(RGB32UI), LOCAL_GL_RGB_INTEGER, LOCAL_GL_UNSIGNED_INT  );
    fnAddSizedUnpack(FOO(RGB32I ), LOCAL_GL_RGB_INTEGER, LOCAL_GL_INT           );

    fnAddSizedUnpack(FOO(RG8      ), LOCAL_GL_RG, LOCAL_GL_UNSIGNED_BYTE);
    fnAddSizedUnpack(FOO(RG8_SNORM), LOCAL_GL_RG, LOCAL_GL_BYTE         );
    fnAddSizedUnpack(FOO(RG16F    ), LOCAL_GL_RG, LOCAL_GL_HALF_FLOAT   );
    fnAddSizedUnpack(FOO(RG16F    ), LOCAL_GL_RG, LOCAL_GL_FLOAT        );
    fnAddSizedUnpack(FOO(RG32F    ), LOCAL_GL_RG, LOCAL_GL_FLOAT        );

    fnAddSizedUnpack(FOO(RG8UI ), LOCAL_GL_RG_INTEGER, LOCAL_GL_UNSIGNED_BYTE );
    fnAddSizedUnpack(FOO(RG8I  ), LOCAL_GL_RG_INTEGER, LOCAL_GL_BYTE          );
    fnAddSizedUnpack(FOO(RG16UI), LOCAL_GL_RG_INTEGER, LOCAL_GL_UNSIGNED_SHORT);
    fnAddSizedUnpack(FOO(RG16I ), LOCAL_GL_RG_INTEGER, LOCAL_GL_SHORT         );
    fnAddSizedUnpack(FOO(RG32UI), LOCAL_GL_RG_INTEGER, LOCAL_GL_UNSIGNED_INT  );
    fnAddSizedUnpack(FOO(RG32I ), LOCAL_GL_RG_INTEGER, LOCAL_GL_INT           );

    fnAddSizedUnpack(FOO(R8      ), LOCAL_GL_RED, LOCAL_GL_UNSIGNED_BYTE);
    fnAddSizedUnpack(FOO(R8_SNORM), LOCAL_GL_RED, LOCAL_GL_BYTE         );
    fnAddSizedUnpack(FOO(R16F    ), LOCAL_GL_RED, LOCAL_GL_HALF_FLOAT   );
    fnAddSizedUnpack(FOO(R16F    ), LOCAL_GL_RED, LOCAL_GL_FLOAT        );
    fnAddSizedUnpack(FOO(R32F    ), LOCAL_GL_RED, LOCAL_GL_FLOAT        );

    fnAddSizedUnpack(FOO(R8UI ), LOCAL_GL_RED_INTEGER, LOCAL_GL_UNSIGNED_BYTE );
    fnAddSizedUnpack(FOO(R8I  ), LOCAL_GL_RED_INTEGER, LOCAL_GL_BYTE          );
    fnAddSizedUnpack(FOO(R16UI), LOCAL_GL_RED_INTEGER, LOCAL_GL_UNSIGNED_SHORT);
    fnAddSizedUnpack(FOO(R16I ), LOCAL_GL_RED_INTEGER, LOCAL_GL_SHORT         );
    fnAddSizedUnpack(FOO(R32UI), LOCAL_GL_RED_INTEGER, LOCAL_GL_UNSIGNED_INT  );
    fnAddSizedUnpack(FOO(R32I ), LOCAL_GL_RED_INTEGER, LOCAL_GL_INT           );

    fnAddSizedUnpack(FOO(DEPTH_COMPONENT16 ), LOCAL_GL_DEPTH_COMPONENT, LOCAL_GL_UNSIGNED_SHORT);
    fnAddSizedUnpack(FOO(DEPTH_COMPONENT16 ), LOCAL_GL_DEPTH_COMPONENT, LOCAL_GL_UNSIGNED_INT  );
    fnAddSizedUnpack(FOO(DEPTH_COMPONENT24 ), LOCAL_GL_DEPTH_COMPONENT, LOCAL_GL_UNSIGNED_INT  );
    fnAddSizedUnpack(FOO(DEPTH_COMPONENT32F), LOCAL_GL_DEPTH_COMPONENT, LOCAL_GL_FLOAT         );

    fnAddSizedUnpack(FOO(DEPTH24_STENCIL8 ), LOCAL_GL_DEPTH_STENCIL, LOCAL_GL_UNSIGNED_INT_24_8             );
    fnAddSizedUnpack(FOO(DEPTH32F_STENCIL8), LOCAL_GL_DEPTH_STENCIL, LOCAL_GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

#undef FOO



    const auto fnAllowES3TexFormat = [ptr](GLenum sizedFormat, EffectiveFormat effFormat,
                                           bool isRenderable, bool isFilterable)
    {
        auto usage = ptr->EditUsage(effFormat);
        usage->isFilterable = isFilterable;

        if (isRenderable) {
            usage->SetRenderable();
        }

        ptr->AllowSizedTexFormat(sizedFormat, usage);

        if (isRenderable) {
            ptr->AllowRBFormat(sizedFormat, usage);
        }
    };

#define FOO(x) LOCAL_GL_ ## x, EffectiveFormat::x

    fnAllowES3TexFormat(FOO(R8         ), true , true );
    fnAllowES3TexFormat(FOO(R8_SNORM   ), false, true );
    fnAllowES3TexFormat(FOO(RG8        ), true , true );
    fnAllowES3TexFormat(FOO(RG8_SNORM  ), false, true );
    fnAllowES3TexFormat(FOO(RGB8       ), true , true );
    fnAllowES3TexFormat(FOO(RGB8_SNORM ), false, true );
    fnAllowES3TexFormat(FOO(RGB565     ), true , true );
    fnAllowES3TexFormat(FOO(RGBA4      ), true , true );
    fnAllowES3TexFormat(FOO(RGB5_A1    ), true , true );
    fnAllowES3TexFormat(FOO(RGBA8      ), true , true );
    fnAllowES3TexFormat(FOO(RGBA8_SNORM), false, true );
    fnAllowES3TexFormat(FOO(RGB10_A2   ), true , true );
    fnAllowES3TexFormat(FOO(RGB10_A2UI ), true , false);

    fnAllowES3TexFormat(FOO(SRGB8       ), false, true);
    fnAllowES3TexFormat(FOO(SRGB8_ALPHA8), true , true);

    fnAllowES3TexFormat(FOO(R16F   ), false, true);
    fnAllowES3TexFormat(FOO(RG16F  ), false, true);
    fnAllowES3TexFormat(FOO(RGB16F ), false, true);
    fnAllowES3TexFormat(FOO(RGBA16F), false, true);

    fnAllowES3TexFormat(FOO(R32F   ), false, false);
    fnAllowES3TexFormat(FOO(RG32F  ), false, false);
    fnAllowES3TexFormat(FOO(RGB32F ), false, false);
    fnAllowES3TexFormat(FOO(RGBA32F), false, false);

    fnAllowES3TexFormat(FOO(R11F_G11F_B10F), false, true);
    fnAllowES3TexFormat(FOO(RGB9_E5       ), false, true);

    fnAllowES3TexFormat(FOO(R8I  ), true, false);
    fnAllowES3TexFormat(FOO(R8UI ), true, false);
    fnAllowES3TexFormat(FOO(R16I ), true, false);
    fnAllowES3TexFormat(FOO(R16UI), true, false);
    fnAllowES3TexFormat(FOO(R32I ), true, false);
    fnAllowES3TexFormat(FOO(R32UI), true, false);

    fnAllowES3TexFormat(FOO(RG8I  ), true, false);
    fnAllowES3TexFormat(FOO(RG8UI ), true, false);
    fnAllowES3TexFormat(FOO(RG16I ), true, false);
    fnAllowES3TexFormat(FOO(RG16UI), true, false);
    fnAllowES3TexFormat(FOO(RG32I ), true, false);
    fnAllowES3TexFormat(FOO(RG32UI), true, false);

    fnAllowES3TexFormat(FOO(RGB8I  ), false, false);
    fnAllowES3TexFormat(FOO(RGB8UI ), false, false);
    fnAllowES3TexFormat(FOO(RGB16I ), false, false);
    fnAllowES3TexFormat(FOO(RGB16UI), false, false);
    fnAllowES3TexFormat(FOO(RGB32I ), false, false);
    fnAllowES3TexFormat(FOO(RGB32UI), false, false);

    fnAllowES3TexFormat(FOO(RGBA8I  ), true, false);
    fnAllowES3TexFormat(FOO(RGBA8UI ), true, false);
    fnAllowES3TexFormat(FOO(RGBA16I ), true, false);
    fnAllowES3TexFormat(FOO(RGBA16UI), true, false);
    fnAllowES3TexFormat(FOO(RGBA32I ), true, false);
    fnAllowES3TexFormat(FOO(RGBA32UI), true, false);

    fnAllowES3TexFormat(FOO(DEPTH_COMPONENT16 ), true, false);
    fnAllowES3TexFormat(FOO(DEPTH_COMPONENT24 ), true, false);
    fnAllowES3TexFormat(FOO(DEPTH_COMPONENT32F), true, false);
    fnAllowES3TexFormat(FOO(DEPTH24_STENCIL8  ), true, false);
    fnAllowES3TexFormat(FOO(DEPTH32F_STENCIL8 ), true, false);

#undef FOO


    auto usage = ptr->EditUsage(EffectiveFormat::STENCIL_INDEX8);
    usage->SetRenderable();
    ptr->AllowRBFormat(LOCAL_GL_STENCIL_INDEX8, usage);


    if (!AddUnsizedFormats(ptr, gl))
        return nullptr;

    ptr->AllowRBFormat(LOCAL_GL_DEPTH_STENCIL,
                       ptr->GetUsage(EffectiveFormat::DEPTH24_STENCIL8));



    return Move(ret);
}


void
FormatUsageAuthority::AddTexUnpack(FormatUsageInfo* usage, const PackingInfo& pi,
                                   const DriverUnpackInfo& dui)
{
    auto res = usage->validUnpacks.insert({ pi, dui });
    auto itr = res.first;

    if (!usage->idealUnpack) {
        usage->idealUnpack = &(itr->second);
    }

    mValidTexUnpackFormats.insert(pi.format);
    mValidTexUnpackTypes.insert(pi.type);
}

static bool
Contains(const std::set<GLenum>& set, GLenum key)
{
    return set.find(key) != set.end();
}

bool
FormatUsageAuthority::IsInternalFormatEnumValid(GLenum internalFormat) const
{
    return Contains(mValidTexInternalFormats, internalFormat);
}

bool
FormatUsageAuthority::AreUnpackEnumsValid(GLenum unpackFormat, GLenum unpackType) const
{
    return (Contains(mValidTexUnpackFormats, unpackFormat) &&
            Contains(mValidTexUnpackTypes, unpackType));
}


void
FormatUsageAuthority::AllowRBFormat(GLenum sizedFormat, const FormatUsageInfo* usage)
{
    MOZ_ASSERT(!usage->format->compression);
    MOZ_ASSERT(usage->format->sizedFormat);
    MOZ_ASSERT(usage->IsRenderable());

    AlwaysInsert(mRBFormatMap, sizedFormat, usage);
}

void
FormatUsageAuthority::AllowSizedTexFormat(GLenum sizedFormat,
                                          const FormatUsageInfo* usage)
{
    if (usage->format->compression) {
        MOZ_ASSERT(usage->isFilterable, "Compressed formats should be filterable.");
    } else {
        MOZ_ASSERT(usage->validUnpacks.size() && usage->idealUnpack,
                   "AddTexUnpack() first.");
    }

    AlwaysInsert(mSizedTexFormatMap, sizedFormat, usage);

    mValidTexInternalFormats.insert(sizedFormat);
}

void
FormatUsageAuthority::AllowUnsizedTexFormat(const PackingInfo& pi,
                                            const FormatUsageInfo* usage)
{
    MOZ_ASSERT(!usage->format->compression);
    MOZ_ASSERT(usage->validUnpacks.size() && usage->idealUnpack, "AddTexUnpack() first.");

    AlwaysInsert(mUnsizedTexFormatMap, pi, usage);

    mValidTexInternalFormats.insert(pi.format);
    mValidTexUnpackFormats.insert(pi.format);
    mValidTexUnpackTypes.insert(pi.type);
}

const FormatUsageInfo*
FormatUsageAuthority::GetRBUsage(GLenum sizedFormat) const
{
    return FindOrNull(mRBFormatMap, sizedFormat);
}

const FormatUsageInfo*
FormatUsageAuthority::GetSizedTexUsage(GLenum sizedFormat) const
{
    return FindOrNull(mSizedTexFormatMap, sizedFormat);
}

const FormatUsageInfo*
FormatUsageAuthority::GetUnsizedTexUsage(const PackingInfo& pi) const
{
    return FindOrNull(mUnsizedTexFormatMap, pi);
}

FormatUsageInfo*
FormatUsageAuthority::EditUsage(EffectiveFormat format)
{
    auto itr = mUsageMap.find(format);

    if (itr == mUsageMap.end()) {
        const FormatInfo* formatInfo = GetFormat(format);
        MOZ_RELEASE_ASSERT(formatInfo, "GFX: no format info set.");

        FormatUsageInfo usage(formatInfo);

        auto res = mUsageMap.insert({ format, usage });

        itr = res.first;
    }

    return &(itr->second);
}

const FormatUsageInfo*
FormatUsageAuthority::GetUsage(EffectiveFormat format) const
{
    auto itr = mUsageMap.find(format);
    if (itr == mUsageMap.end())
        return nullptr;

    return &(itr->second);
}


} // namespace webgl
} // namespace mozilla


#ifndef RAPIDJSON_ENCODINGS_H_
#define RAPIDJSON_ENCODINGS_H_

#include "rapidjson.h"

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4244) // conversion from 'type1' to 'type2', possible loss of data
RAPIDJSON_DIAG_OFF(4702)  // unreachable code
#elif defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
RAPIDJSON_DIAG_OFF(overflow)
#endif

RAPIDJSON_NAMESPACE_BEGIN


/*! \class rapidjson::Encoding
    \brief Concept for encoding of Unicode characters.

\code
concept Encoding {
    typename Ch;    //! Type of character. A "character" is actually a code unit in unicode's definition.

    enum { supportUnicode = 1 }; // or 0 if not supporting unicode

    template<typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint);

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint);

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os);


    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is);

    template <typename InputByteStream>
    static Ch Take(InputByteStream& is);

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os);

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, Ch c);
};
\endcode
*/


/*! http://en.wikipedia.org/wiki/UTF-8
    http://tools.ietf.org/html/rfc3629
    \tparam CharType Code unit for storing 8-bit UTF-8 data. Default is char.
    \note implements Encoding concept
*/
template<typename CharType = char>
struct UTF8 {
    typedef CharType Ch;

    enum { supportUnicode = 1 };

    template<typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint) {
        if (codepoint <= 0x7F) 
            os.Put(static_cast<Ch>(codepoint & 0xFF));
        else if (codepoint <= 0x7FF) {
            os.Put(static_cast<Ch>(0xC0 | ((codepoint >> 6) & 0xFF)));
            os.Put(static_cast<Ch>(0x80 | ((codepoint & 0x3F))));
        }
        else if (codepoint <= 0xFFFF) {
            os.Put(static_cast<Ch>(0xE0 | ((codepoint >> 12) & 0xFF)));
            os.Put(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.Put(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
        else {
            RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
            os.Put(static_cast<Ch>(0xF0 | ((codepoint >> 18) & 0xFF)));
            os.Put(static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
            os.Put(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.Put(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
    }

    template<typename OutputStream>
    static void EncodeUnsafe(OutputStream& os, unsigned codepoint) {
        if (codepoint <= 0x7F) 
            PutUnsafe(os, static_cast<Ch>(codepoint & 0xFF));
        else if (codepoint <= 0x7FF) {
            PutUnsafe(os, static_cast<Ch>(0xC0 | ((codepoint >> 6) & 0xFF)));
            PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint & 0x3F))));
        }
        else if (codepoint <= 0xFFFF) {
            PutUnsafe(os, static_cast<Ch>(0xE0 | ((codepoint >> 12) & 0xFF)));
            PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            PutUnsafe(os, static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
        else {
            RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
            PutUnsafe(os, static_cast<Ch>(0xF0 | ((codepoint >> 18) & 0xFF)));
            PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
            PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            PutUnsafe(os, static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
    }

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint) {
#define COPY() c = is.Take(); *codepoint = (*codepoint << 6) | (static_cast<unsigned char>(c) & 0x3Fu)
#define TRANS(mask) result &= ((GetRange(static_cast<unsigned char>(c)) & mask) != 0)
#define TAIL() COPY(); TRANS(0x70)
        typename InputStream::Ch c = is.Take();
        if (!(c & 0x80)) {
            *codepoint = static_cast<unsigned char>(c);
            return true;
        }

        unsigned char type = GetRange(static_cast<unsigned char>(c));
        if (type >= 32) {
            *codepoint = 0;
        } else {
            *codepoint = (0xFF >> type) & static_cast<unsigned char>(c);
        }
        bool result = true;
        switch (type) {
        case 2: TAIL(); return result;
        case 3: TAIL(); TAIL(); return result;
        case 4: COPY(); TRANS(0x50); TAIL(); return result;
        case 5: COPY(); TRANS(0x10); TAIL(); TAIL(); return result;
        case 6: TAIL(); TAIL(); TAIL(); return result;
        case 10: COPY(); TRANS(0x20); TAIL(); return result;
        case 11: COPY(); TRANS(0x60); TAIL(); TAIL(); return result;
        default: return false;
        }
#undef COPY
#undef TRANS
#undef TAIL
    }

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os) {
#define COPY() os.Put(c = is.Take())
#define TRANS(mask) result &= ((GetRange(static_cast<unsigned char>(c)) & mask) != 0)
#define TAIL() COPY(); TRANS(0x70)
        Ch c;
        COPY();
        if (!(c & 0x80))
            return true;

        bool result = true;
        switch (GetRange(static_cast<unsigned char>(c))) {
        case 2: TAIL(); return result;
        case 3: TAIL(); TAIL(); return result;
        case 4: COPY(); TRANS(0x50); TAIL(); return result;
        case 5: COPY(); TRANS(0x10); TAIL(); TAIL(); return result;
        case 6: TAIL(); TAIL(); TAIL(); return result;
        case 10: COPY(); TRANS(0x20); TAIL(); return result;
        case 11: COPY(); TRANS(0x60); TAIL(); TAIL(); return result;
        default: return false;
        }
#undef COPY
#undef TRANS
#undef TAIL
    }

    static unsigned char GetRange(unsigned char c) {
        static const unsigned char type[] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
            0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
            0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
            0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
            8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
        };
        return type[c];
    }

    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        typename InputByteStream::Ch c = Take(is);
        if (static_cast<unsigned char>(c) != 0xEFu) return c;
        c = is.Take();
        if (static_cast<unsigned char>(c) != 0xBBu) return c;
        c = is.Take();
        if (static_cast<unsigned char>(c) != 0xBFu) return c;
        c = is.Take();
        return c;
    }

    template <typename InputByteStream>
    static Ch Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        return static_cast<Ch>(is.Take());
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(0xEFu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xBBu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xBFu));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, Ch c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(c));
    }
};


/*! http://en.wikipedia.org/wiki/UTF-16
    http://tools.ietf.org/html/rfc2781
    \tparam CharType Type for storing 16-bit UTF-16 data. Default is wchar_t. C++11 may use char16_t instead.
    \note implements Encoding concept

    \note For in-memory access, no need to concern endianness. The code units and code points are represented by CPU's endianness.
    For streaming, use UTF16LE and UTF16BE, which handle endianness.
*/
template<typename CharType = wchar_t>
struct UTF16 {
    typedef CharType Ch;
    RAPIDJSON_STATIC_ASSERT(sizeof(Ch) >= 2);

    enum { supportUnicode = 1 };

    template<typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputStream::Ch) >= 2);
        if (codepoint <= 0xFFFF) {
            RAPIDJSON_ASSERT(codepoint < 0xD800 || codepoint > 0xDFFF); // Code point itself cannot be surrogate pair 
            os.Put(static_cast<typename OutputStream::Ch>(codepoint));
        }
        else {
            RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
            unsigned v = codepoint - 0x10000;
            os.Put(static_cast<typename OutputStream::Ch>((v >> 10) | 0xD800));
            os.Put((v & 0x3FF) | 0xDC00);
        }
    }


    template<typename OutputStream>
    static void EncodeUnsafe(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputStream::Ch) >= 2);
        if (codepoint <= 0xFFFF) {
            RAPIDJSON_ASSERT(codepoint < 0xD800 || codepoint > 0xDFFF); // Code point itself cannot be surrogate pair 
            PutUnsafe(os, static_cast<typename OutputStream::Ch>(codepoint));
        }
        else {
            RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
            unsigned v = codepoint - 0x10000;
            PutUnsafe(os, static_cast<typename OutputStream::Ch>((v >> 10) | 0xD800));
            PutUnsafe(os, (v & 0x3FF) | 0xDC00);
        }
    }

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputStream::Ch) >= 2);
        typename InputStream::Ch c = is.Take();
        if (c < 0xD800 || c > 0xDFFF) {
            *codepoint = static_cast<unsigned>(c);
            return true;
        }
        else if (c <= 0xDBFF) {
            *codepoint = (static_cast<unsigned>(c) & 0x3FF) << 10;
            c = is.Take();
            *codepoint |= (static_cast<unsigned>(c) & 0x3FF);
            *codepoint += 0x10000;
            return c >= 0xDC00 && c <= 0xDFFF;
        }
        return false;
    }

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputStream::Ch) >= 2);
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputStream::Ch) >= 2);
        typename InputStream::Ch c;
        os.Put(static_cast<typename OutputStream::Ch>(c = is.Take()));
        if (c < 0xD800 || c > 0xDFFF)
            return true;
        else if (c <= 0xDBFF) {
            os.Put(c = is.Take());
            return c >= 0xDC00 && c <= 0xDFFF;
        }
        return false;
    }
};

template<typename CharType = wchar_t>
struct UTF16LE : UTF16<CharType> {
    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        CharType c = Take(is);
        return static_cast<uint16_t>(c) == 0xFEFFu ? Take(is) : c;
    }

    template <typename InputByteStream>
    static CharType Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        unsigned c = static_cast<uint8_t>(is.Take());
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 8;
        return static_cast<CharType>(c);
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFEu));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, CharType c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(static_cast<unsigned>(c) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((static_cast<unsigned>(c) >> 8) & 0xFFu));
    }
};

template<typename CharType = wchar_t>
struct UTF16BE : UTF16<CharType> {
    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        CharType c = Take(is);
        return static_cast<uint16_t>(c) == 0xFEFFu ? Take(is) : c;
    }

    template <typename InputByteStream>
    static CharType Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        unsigned c = static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 8;
        c |= static_cast<uint8_t>(is.Take());
        return static_cast<CharType>(c);
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFEu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFFu));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, CharType c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>((static_cast<unsigned>(c) >> 8) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>(static_cast<unsigned>(c) & 0xFFu));
    }
};


/*! http://en.wikipedia.org/wiki/UTF-32
    \tparam CharType Type for storing 32-bit UTF-32 data. Default is unsigned. C++11 may use char32_t instead.
    \note implements Encoding concept

    \note For in-memory access, no need to concern endianness. The code units and code points are represented by CPU's endianness.
    For streaming, use UTF32LE and UTF32BE, which handle endianness.
*/
template<typename CharType = unsigned>
struct UTF32 {
    typedef CharType Ch;
    RAPIDJSON_STATIC_ASSERT(sizeof(Ch) >= 4);

    enum { supportUnicode = 1 };

    template<typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputStream::Ch) >= 4);
        RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
        os.Put(codepoint);
    }

    template<typename OutputStream>
    static void EncodeUnsafe(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputStream::Ch) >= 4);
        RAPIDJSON_ASSERT(codepoint <= 0x10FFFF);
        PutUnsafe(os, codepoint);
    }

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputStream::Ch) >= 4);
        Ch c = is.Take();
        *codepoint = c;
        return c <= 0x10FFFF;
    }

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputStream::Ch) >= 4);
        Ch c;
        os.Put(c = is.Take());
        return c <= 0x10FFFF;
    }
};

template<typename CharType = unsigned>
struct UTF32LE : UTF32<CharType> {
    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        CharType c = Take(is);
        return static_cast<uint32_t>(c) == 0x0000FEFFu ? Take(is) : c;
    }

    template <typename InputByteStream>
    static CharType Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        unsigned c = static_cast<uint8_t>(is.Take());
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 8;
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 16;
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 24;
        return static_cast<CharType>(c);
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFEu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0x00u));
        os.Put(static_cast<typename OutputByteStream::Ch>(0x00u));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, CharType c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(c & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 8) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 16) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 24) & 0xFFu));
    }
};

template<typename CharType = unsigned>
struct UTF32BE : UTF32<CharType> {
    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        CharType c = Take(is);
        return static_cast<uint32_t>(c) == 0x0000FEFFu ? Take(is) : c; 
    }

    template <typename InputByteStream>
    static CharType Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        unsigned c = static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 24;
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 16;
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take())) << 8;
        c |= static_cast<unsigned>(static_cast<uint8_t>(is.Take()));
        return static_cast<CharType>(c);
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(0x00u));
        os.Put(static_cast<typename OutputByteStream::Ch>(0x00u));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFEu));
        os.Put(static_cast<typename OutputByteStream::Ch>(0xFFu));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, CharType c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 24) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 16) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>((c >> 8) & 0xFFu));
        os.Put(static_cast<typename OutputByteStream::Ch>(c & 0xFFu));
    }
};


/*! http://en.wikipedia.org/wiki/ASCII
    \tparam CharType Code unit for storing 7-bit ASCII data. Default is char.
    \note implements Encoding concept
*/
template<typename CharType = char>
struct ASCII {
    typedef CharType Ch;

    enum { supportUnicode = 0 };

    template<typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_ASSERT(codepoint <= 0x7F);
        os.Put(static_cast<Ch>(codepoint & 0xFF));
    }

    template<typename OutputStream>
    static void EncodeUnsafe(OutputStream& os, unsigned codepoint) {
        RAPIDJSON_ASSERT(codepoint <= 0x7F);
        PutUnsafe(os, static_cast<Ch>(codepoint & 0xFF));
    }

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint) {
        uint8_t c = static_cast<uint8_t>(is.Take());
        *codepoint = c;
        return c <= 0X7F;
    }

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os) {
        uint8_t c = static_cast<uint8_t>(is.Take());
        os.Put(static_cast<typename OutputStream::Ch>(c));
        return c <= 0x7F;
    }

    template <typename InputByteStream>
    static CharType TakeBOM(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        uint8_t c = static_cast<uint8_t>(Take(is));
        return static_cast<Ch>(c);
    }

    template <typename InputByteStream>
    static Ch Take(InputByteStream& is) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
        return static_cast<Ch>(is.Take());
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        (void)os;
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, Ch c) {
        RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
        os.Put(static_cast<typename OutputByteStream::Ch>(c));
    }
};


enum UTFType {
    kUTF8 = 0,      //!< UTF-8.
    kUTF16LE = 1,   //!< UTF-16 little endian.
    kUTF16BE = 2,   //!< UTF-16 big endian.
    kUTF32LE = 3,   //!< UTF-32 little endian.
    kUTF32BE = 4    //!< UTF-32 big endian.
};

/*! \note This class can be used with AutoUTFInputtStream and AutoUTFOutputStream, which provides GetType().
*/
template<typename CharType>
struct AutoUTF {
    typedef CharType Ch;

    enum { supportUnicode = 1 };

#define RAPIDJSON_ENCODINGS_FUNC(x) UTF8<Ch>::x, UTF16LE<Ch>::x, UTF16BE<Ch>::x, UTF32LE<Ch>::x, UTF32BE<Ch>::x

    template<typename OutputStream>
    RAPIDJSON_FORCEINLINE static void Encode(OutputStream& os, unsigned codepoint) {
        typedef void (*EncodeFunc)(OutputStream&, unsigned);
        static const EncodeFunc f[] = { RAPIDJSON_ENCODINGS_FUNC(Encode) };
        (*f[os.GetType()])(os, codepoint);
    }

    template<typename OutputStream>
    RAPIDJSON_FORCEINLINE static void EncodeUnsafe(OutputStream& os, unsigned codepoint) {
        typedef void (*EncodeFunc)(OutputStream&, unsigned);
        static const EncodeFunc f[] = { RAPIDJSON_ENCODINGS_FUNC(EncodeUnsafe) };
        (*f[os.GetType()])(os, codepoint);
    }

    template <typename InputStream>
    RAPIDJSON_FORCEINLINE static bool Decode(InputStream& is, unsigned* codepoint) {
        typedef bool (*DecodeFunc)(InputStream&, unsigned*);
        static const DecodeFunc f[] = { RAPIDJSON_ENCODINGS_FUNC(Decode) };
        return (*f[is.GetType()])(is, codepoint);
    }

    template <typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
        typedef bool (*ValidateFunc)(InputStream&, OutputStream&);
        static const ValidateFunc f[] = { RAPIDJSON_ENCODINGS_FUNC(Validate) };
        return (*f[is.GetType()])(is, os);
    }

#undef RAPIDJSON_ENCODINGS_FUNC
};


template<typename SourceEncoding, typename TargetEncoding>
struct Transcoder {
    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool Transcode(InputStream& is, OutputStream& os) {
        unsigned codepoint;
        if (!SourceEncoding::Decode(is, &codepoint))
            return false;
        TargetEncoding::Encode(os, codepoint);
        return true;
    }

    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool TranscodeUnsafe(InputStream& is, OutputStream& os) {
        unsigned codepoint;
        if (!SourceEncoding::Decode(is, &codepoint))
            return false;
        TargetEncoding::EncodeUnsafe(os, codepoint);
        return true;
    }

    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
        return Transcode(is, os);   // Since source/target encoding is different, must transcode.
    }
};

template<typename Stream>
inline void PutUnsafe(Stream& stream, typename Stream::Ch c);

template<typename Encoding>
struct Transcoder<Encoding, Encoding> {
    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool Transcode(InputStream& is, OutputStream& os) {
        os.Put(is.Take());  // Just copy one code unit. This semantic is different from primary template class.
        return true;
    }
    
    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool TranscodeUnsafe(InputStream& is, OutputStream& os) {
        PutUnsafe(os, is.Take());  // Just copy one code unit. This semantic is different from primary template class.
        return true;
    }
    
    template<typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE static bool Validate(InputStream& is, OutputStream& os) {
        return Encoding::Validate(is, os);  // source/target encoding are the same
    }
};

RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__) || defined(_MSC_VER)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_ENCODINGS_H_

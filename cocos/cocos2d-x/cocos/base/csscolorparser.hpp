
#ifndef CSS_COLOR_PARSER_CPP
#define CSS_COLOR_PARSER_CPP

#include <string>
#include <math.h>

namespace CSSColorParser {

struct Color {
    inline Color() {
    }
    inline Color(unsigned char r_, unsigned char g_, unsigned char b_, float a_)
        : r(r_), g(g_), b(b_), a(a_ > 1 ? 1 : a_ < 0 ? 0 : a_) {
    }
    unsigned char r = 0, g = 0, b = 0;
    float a = 1.0f;
};

inline bool operator==(const Color& lhs, const Color& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && ::fabs(lhs.a - rhs.a) < 0.0001f;
}

inline bool operator!=(const Color& lhs, const Color& rhs) {
    return !(lhs == rhs);
}

Color parse(const std::string& css_str);

} // namespace CSSColorParser

#endif

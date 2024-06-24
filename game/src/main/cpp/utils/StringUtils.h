//
// Created by jesse on 2024/4/7.
//

#ifndef JNI_STRINGUTILS_H
#define JNI_STRINGUTILS_H

#include <jni.h>
#include <string>

class StringUtils {
public:

    /**
     * javaClassName转C++ClassName
     * (把字符串中的 . 替换成 /)
     * @param str  javaClassName
     * @return C++ ClassName
     */
    static std::string jName2cName(std::string className) {
        std::replace(className.begin(), className.end(), '.', '/');
        return className;
    }

    /**
     * C++字符串转Java字符串
     * @param env
     * @param str  C++字符串
     * @return Java字符串
     */
    static jstring string2jString(JNIEnv *env, std::string str) {
        return env->NewStringUTF(str.c_str());
    }

    /**
     * Java字符串转C++字符串
     * @param env
     * @param javaString Java字符串
     * @return C++字符串
     */
    static std::string jString2string(JNIEnv *env, jstring javaString) {
        if (javaString == nullptr) {
            return "";
        }
        const char *str = env->GetStringUTFChars(javaString, nullptr);
        std::string cString(str);
        //释放资源
        env->ReleaseStringUTFChars(javaString, str);

        return cString;
    }

    /**
     * 判断a字符串是否相等b字符串 （不忽略大小写）
     * @param a 字符串
     * @param b 字符串
     * @return 是否相等
     */
    static bool equals(std::string a, std::string b) {
        return a == b;
    }

    /**
     * 判断a字符串是否相等b字符串 （忽略大小写）
     * @param a 字符串
     * @param b 字符串
     * @return 是否相等
     */
    static bool equalsIgnoreCase(std::string a, std::string b) {
        if (a.length() != b.length())
            return false;

        return toLower(a) == toLower(b);
    }

    /**
     * 判断a字符串是否包含b字符串
     * @param a 字符串
     * @param b 字符串
     * @return 是否包含
     */
    static bool contains(std::string a, std::string b) {
        return a.find(b) != std::string::npos;
    }

    /**
     * 判断a字符串是否包含b字符串（忽略大小写）
     * @param a 字符串
     * @param b 字符串
     * @return 是否包含
     */
    static bool containsIgnoreCase(std::string a, std::string b) {
        return toLower(a).find(toLower(b)) != std::string::npos;
    }

    /**
     * 字符串转大写
     *
     * @param str 字符串
     * @return 大写字符串
     */
    static std::string toUpper(std::string str) {
        if (str.empty())
            return str;

        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
        return str;
    }

    /**
     * 字符串转小写
     *
     * @param str 字符串
     * @return 小写字符串
     */
    static std::string toLower(std::string str) {
        if (str.empty())
            return str;

        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return str;
    }

    /**
     * 字符串首字母转大写
     *
     * @param str 字符串
     * @return 首字大写字符串
     */
    static std::string upperFirstLetter(std::string str) {
        if (str.empty())
            return str;

        str[0] = std::toupper(str[0]);
        return str;
    }

    /**
     * 字符串首字母转小写
     *
     * @param str 字符串
     * @return 首字母小写字符串
     */
    static std::string lowerFirstLetter(std::string str) {
        if (str.empty())
            return str;

        str[0] = std::tolower(str[0]);
        return str;
    }
};

#endif //JNI_STRINGUTILS_H

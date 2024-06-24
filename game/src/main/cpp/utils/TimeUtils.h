//
// Created by jesse on 2024/4/11.
//

#ifndef JNI_TIMEUTILS_H
#define JNI_TIMEUTILS_H

#include <jni.h>
#include <chrono>
#include <string>

class TimeUtils {
public:
    /**
     * 获取当前时间戳（毫秒）
     *
     * @return 时间戳（毫秒）
     */
    static long long getCurrentTimeMillis() {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        // 将时间戳转换为毫秒数
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        return now_ms.time_since_epoch().count();
    }

    /**
     * 时间戳（毫秒）格式化（TimeUtils::millis2String(1712822935524, "%Y-%m-%d %H:%M:%S")）
     *
     * @param time 毫秒时间戳
     * @param format 格式
     * @return 格式字符串
     */
    static std::string millis2String(long long time, const char *format) {
        // 时间戳转换为 std::time_t 类型
        std::time_t t = static_cast<std::time_t>(time / 1000);
        // 使用 std::tm 结构体存储日期时间信息
        std::tm *tm_time = std::localtime(&t);
        // 将日期时间格式化为字符串
        char buffer[100]; // 适当大小的缓冲区
        std::strftime(buffer, sizeof(buffer), format, tm_time);
        return std::string(buffer);
    }

    /**
     * 时间字符串转换为毫秒
     *
     * @param time 时间字符串
     * @param format 格式
     * @return 时间戳毫秒
     */
    static long long string2Millis(const char *time, const char *format) {
        std::tm tm_time = {};
        // 从字符串中解析日期时间信息
        if (strptime(time, format, &tm_time) == nullptr) {
            return -1; // 解析失败，返回-1
        }
        // 将日期时间转换为 std::time_t 类型
        std::time_t t = std::mktime(&tm_time);
        // 如果解析失败，则返回-1
        if (t == -1) {
            return -1;
        }
        return static_cast<long long>(t) * 1000;
    }
};


#endif //JNI_TIMEUTILS_H

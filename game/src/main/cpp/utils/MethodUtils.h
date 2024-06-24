#ifndef JNI_MEHTODUTILS_H
#define JNI_MEHTODUTILS_H

#include <jni.h>
#include <string>

class MethodUtils {
public:
    /**
     * 调用java有返回值的方法
     *
     * @param env
     * @param obj java对象
     * @param methodName java方法名
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)Ljava/lang/String;）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     * @return 方法返回数据
     */
    template<typename T>
    static T callMethod(JNIEnv *env, jobject obj, const char *methodName, const char *methodSig,
                        ...) {
        va_list args;
        //获取class
        jclass objClass = env->GetObjectClass(obj);
        //获取方法ID
        jmethodID methodID = env->GetMethodID(objClass, methodName, methodSig);
        va_start(args, methodSig);
        //调用方法
        T result = callMethodType<T>(env, obj, methodID, args);
        va_end(args);
        // 释放资源
        env->DeleteLocalRef(objClass);
        return result;
    }

    /**
     * 调用java有返回值的静态方法
     *
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param methodName java方法名
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)Ljava/lang/String;）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     * @return 方法返回数据
     */
    template<typename T>
    static T callStaticMethod(JNIEnv *env, const char *className, const char *methodName,
                              const char *methodSig, ...) {
        va_list args;
        //获取class
        jclass objClass = env->FindClass(className);
        //获取方法ID
        jmethodID methodID = env->GetStaticMethodID(objClass, methodName, methodSig);
        va_start(args, methodSig);
        //调用方法
        T result = callStaticMethodType<T>(env, objClass, methodID, args);
        va_end(args);
        // 释放资源
        env->DeleteLocalRef(objClass);
        return result;
    }


    /**
     * 调用Kotlin有返回值的静态方法
     *
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param methodName java方法名
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)Ljava/lang/String;）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     * @return 方法返回数据
     */
    template<typename T>
    static T callKotlinStaticMethod(JNIEnv *env, const char *className, const char *methodName,
                                    const char *methodSig, ...) {
        va_list args;
        std::string sig = "L";
        sig.append(className).append(";");
        //获取静态对象
        jobject obj = FieldUtils::getStaticField<jobject>(env, className,
                                                          "INSTANCE", sig.c_str());
        //获取方法ID
        jmethodID methodID = env->GetMethodID(ClassUtils::getClass(env, obj), methodName,
                                              methodSig);
        va_start(args, methodSig);
        //调用方法
        T result = callMethodType<T>(env, obj, methodID, args);
        va_end(args);
        // 释放资源
        env->DeleteLocalRef(obj);
        return result;
    }

    /**
     * 调用java带没有返回值的方法
     *
     * @param env
     * @param obj java对象
     * @param methodName java方法名
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)V）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     */
    static void callVoidMethod(JNIEnv *env, jobject obj, const char *methodName,
                               const char *methodSig, ...) {
        va_list args;
        //获取class
        jclass objClass = env->GetObjectClass(obj);
        //获取方法ID
        jmethodID methodID = env->GetMethodID(objClass, methodName, methodSig);
        va_start(args, methodSig);
        //调用方法
        env->CallVoidMethodV(obj, methodID, args);
        va_end(args);
        // 释放资源
        env->DeleteLocalRef(objClass);
    }

    /**
     * 调用Kotlin没有返回值的静态方法
     *
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param methodName java方法名
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)V）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     */
    static void callStaticVoidMethod(JNIEnv *env, const char *className, const char *methodName,
                                     const char *methodSig, ...) {
        va_list args;
        //获取class
        std::string sig = "L";
        sig.append(className).append(";");
        //获取静态对象
        jobject obj = FieldUtils::getStaticField<jobject>(env, className,
                                                          "INSTANCE", sig.c_str());
        //获取方法ID
        jmethodID methodID = env->GetMethodID(ClassUtils::getClass(env, obj), methodName, methodSig);
        va_start(args, methodSig);
        //调用方法
        env->CallVoidMethodV(obj, methodID, args);
        va_end(args);
        // 释放资源
        env->DeleteLocalRef(obj);
    }

private:

    // -------------------------  调用方法获取返回值  -------------------------
    template<typename T>
    static T callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }

    template<>
    jstring callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return (jstring) env->CallObjectMethodV(obj, methodID, args);
    }

    template<>
    jobject callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallObjectMethodV(obj, methodID, args);
    }

    template<>
    jint callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallIntMethodV(obj, methodID, args);
    }

    template<>
    jboolean callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallBooleanMethodV(obj, methodID, args);
    }

    template<>
    jbyte callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallByteMethodV(obj, methodID, args);
    }

    template<>
    jchar callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallCharMethodV(obj, methodID, args);
    }

    template<>
    jshort callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallShortMethodV(obj, methodID, args);
    }

    template<>
    jlong callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallLongMethodV(obj, methodID, args);
    }

    template<>
    jfloat callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallFloatMethodV(obj, methodID, args);
    }

    template<>
    jdouble callMethodType(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
        return env->CallDoubleMethodV(obj, methodID, args);
    }


    // -------------------------  调用静态方法获取返回值  -------------------------

    template<typename T>
    static T callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }


    template<>
    jstring
    callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return (jstring) env->CallStaticObjectMethodV(clazz, methodID, args);
    }

    template<>
    jobject
    callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticObjectMethodV(clazz, methodID, args);
    }

    template<>
    jint callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticIntMethodV(clazz, methodID, args);
    }

    template<>
    jboolean callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticBooleanMethodV(clazz, methodID, args);
    }

    template<>
    jbyte callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticByteMethodV(clazz, methodID, args);
    }

    template<>
    jchar callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticCharMethodV(clazz, methodID, args);
    }

    template<>
    jshort callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticShortMethodV(clazz, methodID, args);
    }

    template<>
    jlong callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticLongMethodV(clazz, methodID, args);
    }

    template<>
    jfloat callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticFloatMethodV(clazz, methodID, args);
    }

    template<>
    jdouble callStaticMethodType(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
        return env->CallStaticDoubleMethodV(clazz, methodID, args);
    }

};

#endif //JNI_MEHTODUTILS_H
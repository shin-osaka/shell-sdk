//
// Created by jesse on 2024/4/7.
//
#ifndef JNI_FIELDUTILS_H
#define JNI_FIELDUTILS_H

#include <jni.h>
#include <stdexcept>

class FieldUtils {
public:
    /**
     * 获取java变量值
     *
     * @param env
     * @param thiz java对象
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param fieldName java变量名
     * @param fieldSig 变量类型声明 （Ljava/lang/String;）
     * @return 变量值
     */
    template<typename T>
    static T getField(JNIEnv *env, jobject thiz,
                      const char *fieldName, const char *fieldSig) {
        //获取class
        jclass clazz = env->GetObjectClass(thiz);
        //获取变量ID
        jfieldID fieldId = env->GetFieldID(clazz, fieldName, fieldSig);
        //获取变量
        T obj = getFieldType<T>(env, thiz, fieldId);
        // 释放资源
        env->DeleteLocalRef(clazz);
        return obj;
    }

    /**
     * 获取java静态变量值
     *
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param fieldName java变量名
     * @param fieldSig 变量类型声明 （Ljava/lang/String;）
     * @return 变量值
     */
    template<typename T>
    static T getStaticField(JNIEnv *env, const char *className, const char *fieldName,
                            const char *fieldSig) {
        //获取class
        jclass clazz = env->FindClass(className);
        //获取变量ID
        jfieldID fieldId = env->GetStaticFieldID(clazz, fieldName, fieldSig);
        //获取变量
        T obj = getStaticFieldType<T>(env, clazz, fieldId);
        // 释放资源
        env->DeleteLocalRef(clazz);
        return obj;
    }

    /**
  * 获取java静态变量值
  *
  * @param env
  * @param className java包名+类名 （xxx/xxx/xxxx/类名）
  * @param fieldName java变量名
  * @param fieldSig 变量类型声明 （Ljava/lang/String;）
  * @return 变量值
  */
    template<typename T>
    static T getStaticField(JNIEnv *env,  jobject thiz, const char *fieldName,
                            const char *fieldSig) {
        //获取class
        jclass clazz = env->GetObjectClass(thiz);
        //获取变量ID
        jfieldID fieldId = env->GetStaticFieldID(clazz, fieldName, fieldSig);
        //获取变量
        T obj = getStaticFieldType<T>(env, clazz, fieldId);
        // 释放资源
        env->DeleteLocalRef(clazz);
        return obj;
    }

    /**
     * 设置java变量值
     *
     * @param env
     * @param thiz java对象
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param fieldName java变量名
     * @param fieldSig 变量类型声明 （Ljava/lang/String;）
     * @param value 设置变量的值 （env->NewStringUTF("String")）
     */
    template<typename T>
    static void setField(JNIEnv *env, jobject thiz, const char *fieldName,
                         const char *fieldSig, T value) {
        //获取class
        jclass clazz = env->GetObjectClass(thiz);
        //获取变量ID
        jfieldID fieldId = env->GetFieldID(clazz, fieldName, fieldSig);
        //设置变量
        setFieldType<T>(env, thiz, fieldId, value);
        // 释放资源
        env->DeleteLocalRef(clazz);
    }

    /**
     * 设置java静态变量值
     *
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param fieldName java变量名
     * @param fieldSig 变量类型声明 （Ljava/lang/String;）
     * @param value 设置变量的值 （env->NewStringUTF("String")）
     */
    template<typename T>
    static void setStaticField(JNIEnv *env, const char *className, const char *fieldName,
                               const char *fieldSig, T value) {
        //获取class
        jclass clazz = env->FindClass(className);
        //获取变量ID
        jfieldID fieldId = env->GetStaticFieldID(clazz, fieldName, fieldSig);
        //设置变量
        setStaticFieldType<T>(env, clazz, fieldId, value);
        // 释放资源
        env->DeleteLocalRef(clazz);
    }


private:

    // -------------------------  获取变量  -------------------------
    template<typename T>
    static T getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }

    template<>
    jstring getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return (jstring) env->GetObjectField(obj, fieldId);
    }

    template<>
    jobject getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetObjectField(obj, fieldId);
    }

    template<>
    jint getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetIntField(obj, fieldId);
    }

    template<>
    jboolean getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetBooleanField(obj, fieldId);
    }

    template<>
    jbyte getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetByteField(obj, fieldId);
    }

    template<>
    jchar getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetCharField(obj, fieldId);
    }

    template<>
    jshort getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetShortField(obj, fieldId);
    }

    template<>
    jlong getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetLongField(obj, fieldId);
    }

    template<>
    jfloat getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetFloatField(obj, fieldId);
    }

    template<>
    jdouble getFieldType(JNIEnv *env, jobject obj, jfieldID fieldId) {
        return env->GetDoubleField(obj, fieldId);
    }


    // -------------------------  获取静态变量  -------------------------
    template<typename T>
    static T getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }

    template<>
    jstring getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return (jstring) env->GetStaticObjectField(clazz, fieldId);
    }

    template<>
    jobject getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticObjectField(clazz, fieldId);
    }

    template<>
    jint getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticIntField(clazz, fieldId);
    }

    template<>
    jboolean getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticBooleanField(clazz, fieldId);
    }

    template<>
    jbyte getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticByteField(clazz, fieldId);
    }

    template<>
    jchar getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticCharField(clazz, fieldId);
    }

    template<>
    jshort getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticShortField(clazz, fieldId);
    }

    template<>
    jlong getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticLongField(clazz, fieldId);
    }

    template<>
    jfloat getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticFloatField(clazz, fieldId);
    }

    template<>
    jdouble getStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId) {
        return env->GetStaticDoubleField(clazz, fieldId);
    }


    // -------------------------  设置变量 -------------------------
    template<typename T>
    static void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, T value) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jstring value) {
        env->SetObjectField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jobject value) {
        env->SetObjectField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jint value) {
        env->SetIntField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jboolean value) {
        env->SetBooleanField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jbyte value) {
        env->SetByteField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jchar value) {
        env->SetCharField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jshort value) {
        env->SetShortField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jlong value) {
        env->SetLongField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jfloat value) {
        env->SetFloatField(obj, fieldId, value);
    }

    template<>
    void setFieldType(JNIEnv *env, jobject obj, jfieldID fieldId, jdouble value) {
        env->SetDoubleField(obj, fieldId, value);
    }


    // -------------------------  设置静态变量 -------------------------
    template<typename T>
    static void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, T value) {
        throw std::runtime_error("Specified type method not found, please use <jobject>");
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jstring value) {
        env->SetStaticObjectField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jobject value) {
        env->SetStaticObjectField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jint value) {
        env->SetStaticIntField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jboolean value) {
        env->SetStaticBooleanField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jbyte value) {
        env->SetStaticByteField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jchar value) {
        env->SetStaticCharField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jshort value) {
        env->SetStaticShortField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jlong value) {
        env->SetStaticLongField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jfloat value) {
        env->SetStaticFloatField(clazz, fieldId, value);
    }

    template<>
    void setStaticFieldType(JNIEnv *env, jclass clazz, jfieldID fieldId, jdouble value) {
        env->SetStaticDoubleField(clazz, fieldId, value);
    }
};
#endif //JNI_FIELDUTILS_H
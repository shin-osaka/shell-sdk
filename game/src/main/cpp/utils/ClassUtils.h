//
// Created by jesse on 2024/4/7.
//

#ifndef JNI_CLASSUTILS_H
#define JNI_CLASSUTILS_H

#include <jni.h>

class ClassUtils {
public:
    /**
     * 创建java对象，无参构造
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @return java对象
     */
    static jobject getInstance(JNIEnv *env, const char *className) {
        try {
            //获取class对象
            jclass clazz = getClass(env, className);
            if (clazz != nullptr) {
                //获取构造函数ID
                jmethodID methodID = env->GetMethodID(clazz, "<init>", "()V");
                //调用构造方法创建java对象
                jobject obj = env->NewObject(clazz, methodID);
                //释放资源
                env->DeleteLocalRef(clazz);
                //返回对象
                return obj;
            }
        } catch (const std::exception &e) {

        }
        return nullptr;
    }

    /**
     * 创建java对象，有参构造
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @param methodSig 方法参数及返回值声明 （(ILjava/lang/String;)V）
     * @param args 方法参数值值 （1213，env->NewStringUTF("String")）
     * @return java对象
     */
    static jobject getInstance(JNIEnv *env, const char *className, const char *methodSig, ...) {
        va_list args;
        //获取class对象
        jclass clazz = getClass(env, className);
        //获取构造函数ID
        jmethodID methodID = env->GetMethodID(clazz, "<init>", methodSig);
        va_start(args, methodSig);
        //调用构造方法创建java对象
        jobject obj = env->NewObjectV(clazz, methodID, args);
        va_end(args);
        //释放资源
        env->DeleteLocalRef(clazz);
        //返回对象
        return obj;
    }

    /**
     * 获取Java Class
     * @param env
     * @param className java包名+类名 （xxx/xxx/xxxx/类名）
     * @return Java Class
     */
    static jclass getClass(JNIEnv *env, const char *className) {
        try {
            jclass cc = env->FindClass(className);
            if (env->ExceptionCheck()) {
                // 异常发生，可以在这里进行处理
                env->ExceptionClear(); // 清除异常状态
            }
            return cc;
        } catch (const std::exception &e) {

        }
        return nullptr;
    }

    /**
     * 获取Java Class
     * @param env
     * @param obj java对象
     * @return Java Class
     */
    static jclass getClass(JNIEnv *env, jobject obj) {
        return env->GetObjectClass(obj);
    }
};

#endif //JNI_CLASSUTILS_H

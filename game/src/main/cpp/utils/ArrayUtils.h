//
// Created by jesse on 2024/4/9.
//

#ifndef JNI_ARRAYUTILS_H
#define JNI_ARRAYUTILS_H

#include <jni.h>
#include <vector>
#include "ClassUtils.h"
#include "StringUtils.h"

class ArrayUtils {
public:
    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java int数组
     * @return C++ int动态数组
     */
    static std::vector<int> jArray2vector(JNIEnv *env, jintArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<int> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jint *elements = env->GetIntArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        // 释放 Java 数组元素的内存
        env->ReleaseIntArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java short数组
     * @return C++ short动态数组
     */
    static std::vector<short> jArray2vector(JNIEnv *env, jshortArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<short> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jshort *elements = env->GetShortArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        // 释放 Java 数组元素的内存
        env->ReleaseShortArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java long数组
     * @return C++ long动态数组
     */
    static std::vector<long> jArray2vector(JNIEnv *env, jlongArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<long> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jlong *elements = env->GetLongArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        // 释放 Java 数组元素的内存
        env->ReleaseLongArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java float数组
     * @return C++ float动态数组
     */
    static std::vector<float> jArray2vector(JNIEnv *env, jfloatArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<float> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jfloat *elements = env->GetFloatArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        // 释放 Java 数组元素的内存
        env->ReleaseFloatArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java double数组
     * @return C++ double动态数组
     */
    static std::vector<double> jArray2vector(JNIEnv *env, jdoubleArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<double> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jdouble *elements = env->GetDoubleArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        // 释放 Java 数组元素的内存
        env->ReleaseDoubleArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java boolean数组
     * @return C++ boolean动态数组
     */
    static std::vector<bool> jArray2vector(JNIEnv *env, jbooleanArray jArray) {
        jsize size = env->GetArrayLength(jArray);
        std::vector<bool> array(size);
        jboolean *elements = env->GetBooleanArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = elements[i];
        }
        env->ReleaseBooleanArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java char数组
     * @return C++ char动态数组
     */
    static std::vector<char> jArray2vector(JNIEnv *env, jcharArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<char> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        jchar *elements = env->GetCharArrayElements(jArray, nullptr);
        for (int i = 0; i < size; ++i) {
            array[i] = static_cast<char>(elements[i]);
        }
        // 释放 Java 数组元素的内存
        env->ReleaseCharArrayElements(jArray, elements, JNI_ABORT);
        return array;
    }

    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java String数组
     * @return C++ String动态数组
     */
    static std::vector<std::string> jStringArray2vector(JNIEnv *env, jobjectArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<std::string> array(size);
        // 将 Java 数组的元素复制到 std::vector 中

        for (int i = 0; i < size; ++i) {
            jstring javaString = (jstring) env->GetObjectArrayElement(jArray, i);
            std::string str = StringUtils::jString2string(env, javaString);
            array[i] = str;
            // 释放资源
            env->DeleteLocalRef(javaString);
        }
        return array;
    }


    /**
     * Java数组转C++动态数组
     * @param env
     * @param jArray Java Object数组
     * @return C++ String动态数组
     */
    static std::vector<jobject> jArray2vector(JNIEnv *env, jobjectArray jArray) {
        // 获取 Java 数组的长度
        jsize size = env->GetArrayLength(jArray);
        // 创建 std::vector
        std::vector<jobject> array(size);
        // 将 Java 数组的元素复制到 std::vector 中
        for (int i = 0; i < size; ++i) {
            jobject obj =  env->GetObjectArrayElement(jArray, i);
            array[i] = obj;
            // 释放资源
            env->DeleteLocalRef(obj);
        }
        return array;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ int动态数组
     * @return Java数组
     */
    static jintArray vector2jArray(JNIEnv *env, std::vector<int> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jintArray javaArray = env->NewIntArray(size);
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetIntArrayRegion(javaArray, 0, size, reinterpret_cast<jint *>(array.data()));
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ short动态数组
     * @return Java数组
     */
    static jshortArray vector2jArray(JNIEnv *env, std::vector<short> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jshortArray javaArray = env->NewShortArray(size);
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetShortArrayRegion(javaArray, 0, size, reinterpret_cast<jshort *>(array.data()));
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ long动态数组
     * @return Java数组
     */
    static jlongArray vector2jArray(JNIEnv *env, std::vector<long> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jlongArray javaArray = env->NewLongArray(size);
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetLongArrayRegion(javaArray, 0, size, reinterpret_cast<jlong *>(array.data()));

        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ float动态数组
     * @return Java数组
     */
    static jfloatArray vector2jArray(JNIEnv *env, std::vector<float> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jfloatArray javaArray = env->NewFloatArray(size);
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetFloatArrayRegion(javaArray, 0, size, reinterpret_cast<jfloat *>(array.data()));
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ double动态数组
     * @return Java数组
     */
    static jdoubleArray vector2jArray(JNIEnv *env, std::vector<double> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jdoubleArray javaArray = env->NewDoubleArray(size);
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetDoubleArrayRegion(javaArray, 0, size, reinterpret_cast<jdouble *>(array.data()));
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ boolean动态数组
     * @return Java数组
     */
    static jbooleanArray vector2jArray(JNIEnv *env, std::vector<bool> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jbooleanArray javaArray = env->NewBooleanArray(size);
        // 创建一个存储jboolean的临时数组
        std::vector<jboolean> tempArray(size);
        for (int i = 0; i < size; ++i) {
            tempArray[i] = array[i] ? JNI_TRUE : JNI_FALSE;
        }
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetBooleanArrayRegion(javaArray, 0, size, tempArray.data());
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ char动态数组
     * @return Java数组
     */
    static jcharArray vector2jArray(JNIEnv *env, std::vector<char> array) {
        //获取数组长度
        int size = array.size();
        //创建java数组
        jcharArray javaArray = env->NewCharArray(size);
        jchar *tempArray = new jchar[size];
        for (int i = 0; i < size; ++i) {
            tempArray[i] = static_cast<jchar>(array[i]);
        }
        // 将 C++ 数组的内容复制到 Java 数组中
        env->SetCharArrayRegion(javaArray, 0, size, tempArray);
        // 释放临时资源
        delete[] tempArray;

        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ String动态数组
     * @return Java数组
     */
    static jobjectArray vector2jArray(JNIEnv *env, std::vector<std::string> array) {
        //获取数组长度
        int size = array.size();
        //获取java String class
        jclass stringClass = ClassUtils::getClass(env, "java/lang/String");
        //创建java数组
        jobjectArray javaArray = env->NewObjectArray(size, stringClass, nullptr);
        for (int i = 0; i < size; ++i) {
            // 获取数据转 java String
            jstring str = StringUtils::string2jString(env, array[i]);
            // 将 C++ 数组的内容复制到 Java 数组中
            env->SetObjectArrayElement(javaArray, i, str);
            //释放资源
            env->DeleteLocalRef(str);
        }
        env->DeleteLocalRef(stringClass);
        return javaArray;
    }

    /**
     * C++动态数组转Java数组
     * @param env
     * @param jArray C++ Object动态数组
     * @return Java数组
     */
    static jobjectArray vector2jArray(JNIEnv *env, std::vector<jobject> array) {
        //获取数组长度
        int size = array.size();
        //获取java Object class
        jclass stringClass = ClassUtils::getClass(env, "java/lang/Object");
        //创建java数组
        jobjectArray javaArray = env->NewObjectArray(size, stringClass, nullptr);
        for (int i = 0; i < size; ++i) {
            // 获取数据转 java String
            jobject obj = array[i];
            // 将 C++ 数组的内容复制到 Java 数组中
            env->SetObjectArrayElement(javaArray, i, obj);
            //释放资源
            env->DeleteLocalRef(obj);
        }
        env->DeleteLocalRef(stringClass);
        return javaArray;
    }
};


#endif //JNI_ARRAYUTILS_H

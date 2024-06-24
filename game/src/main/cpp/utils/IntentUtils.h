//
// Created by jesse on 2024/4/24.
//

#ifndef JNIDEMO_INTENTUTILS_H
#define JNIDEMO_INTENTUTILS_H

#include <jni.h>
#include "ClassUtils.h"
#include "FieldUtils.h"
#include "MethodUtils.h"

class IntentUtils {
public:
    /**
   * 获取 Intent
   *
   * @param env
   * @param context Context 对象
   * @param className 跳转的Activity名（xxx/xxx/xxxx/类名）
   * @return Intent
   */
    static jobject getIntent(JNIEnv *env, jobject context, const char *className) {
        return getIntent(env, context, className, false);
    }

    /**
     * 获取 Intent
     *
     * @param env
     * @param context Context 对象
     * @param className 跳转的Activity名（xxx/xxx/xxxx/类名）
     * @param isNewTask 是否设置FLAG_ACTIVITY_NEW_TASK
     * @return Intent
     */
    static jobject getIntent(JNIEnv *env, jobject context, const char *className, bool isNewTask) {
        //获取Activity Class
        jclass activity = ClassUtils::getClass(env, className);
        //创建Intent
        jobject intent = ClassUtils::getInstance(env, "android/content/Intent",
                                                 "(Landroid/content/Context;Ljava/lang/Class;)V",
                                                 context, activity);
        if (isNewTask) {
            jint FLAG_ACTIVITY_NEW_TASK = FieldUtils::getStaticField<jint>(env,
                                                                           "android/content/Intent",
                                                                           "FLAG_ACTIVITY_NEW_TASK",
                                                                           "I");
            MethodUtils::callMethod<jobject>(env, intent, "setFlags", "(I)Landroid/content/Intent;",
                                             FLAG_ACTIVITY_NEW_TASK);
        }
        return intent;
    }

    /**
     * 添加 Intent
     *
     * @param env
     * @param intent Intent对象
     * @param targetIntent 添加的Intent对象
     */
    static void putIntentExtras(JNIEnv *env, jobject intent, jobject targetIntent) {
        if (targetIntent != nullptr) {
            MethodUtils::callMethod<jobject>(env, intent, "putExtras",
                                             "(Landroid/content/Intent;)Landroid/content/Intent;",
                                             targetIntent);
        }
    }

    /**
     * 添加 Bundle
     *
     * @param env
     * @param intent Intent对象
     * @param bundle 添加的Bundle 对象
     */
    static void putBundleExtras(JNIEnv *env, jobject intent, jobject bundle) {
        if (bundle != nullptr) {
            MethodUtils::callMethod<jobject>(env, intent, "putExtras",
                                             "(Landroid/os/Bundle;)Landroid/content/Intent;",
                                             bundle);
        }
    }

    /**
     * 添加 char
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jchar
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jchar value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;C)Landroid/content/Intent;",
                                         key, value);
    }

    /**
     * 添加 String
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jstring
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jstring value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                         key, value);
    }

    /**
     * 添加 boolean
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jboolean
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jboolean value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;Z)Landroid/content/Intent;",
                                         key, value);
    }

    /**
     * 添加 byte
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jbyte
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jbyte value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;B)Landroid/content/Intent;",
                                         key, value);
    }

    /**
     * 添加 short
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jshort
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jshort value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;S)Landroid/content/Intent;",
                                         key, value);
    }
    /**
     * 添加 int
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jint
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jint value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;I)Landroid/content/Intent;",
                                         key, value);
    }

    /**
     * 添加 long
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jlong
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jlong value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;J)Landroid/content/Intent;",
                                         key, value);
    }
    /**
     * 添加 float
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jfloat
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jfloat value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;F)Landroid/content/Intent;",
                                         key, value);
    }


    /**
     * 添加 double
     *
     * @param env
     * @param intent Intent对象
     * @param key jstring
     * @param value jdouble
     */
    static void putExtra(JNIEnv *env, jobject intent, jstring key, jdouble value) {
        MethodUtils::callMethod<jobject>(env, intent, "putExtra",
                                         "(Ljava/lang/String;D)Landroid/content/Intent;",
                                         key, value);
    }

};

#endif //JNIDEMO_INTENTUTILS_H

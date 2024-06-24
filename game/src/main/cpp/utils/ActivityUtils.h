//
// Created by jesse on 2024/4/15.
//

#ifndef JNI_ACTIVITYUTILS_H
#define JNI_ACTIVITYUTILS_H

#include <jni.h>
#include <map>
#include "ClassUtils.h"
#include "FieldUtils.h"
#include "MethodUtils.h"
#include "IntentUtils.h"

class ActivityUtils {
public:
    /**
     * 启动Activity
     *
     * @param env
     * @param context Context 对象
     * @param className 跳转的Activity名（xxx/xxx/xxxx/类名）
     */
    static void startActivity(JNIEnv *env, jobject context, const char *className) {
        startActivity(env, context, className, false);
    }

    /**
     * 启动Activity
     *
     * @param env
     * @param context Context 对象
     * @param className 跳转的Activity名（xxx/xxx/xxxx/类名）
     * @param isNewTask 是否设置FLAG_ACTIVITY_NEW_TASK
     */
    static void startActivity(JNIEnv *env, jobject context, const char *className, bool isNewTask) {
        //获取 Intent
        jobject intent = IntentUtils::getIntent(env, context, className, isNewTask);
        //启动Activity
        startActivity(env, context,intent);
    }

    /**
     * 启动Activity
     *
     * @param env
     * @param context Context 对象
     * @param intent 启动的Intent
     */
    static void startActivity(JNIEnv *env, jobject context, jobject intent) {
        //启动Activity
        MethodUtils::callVoidMethod(env, context, "startActivity", "(Landroid/content/Intent;)V",
                                    intent);
    }

    /**
     * 关闭Activity
     *
     * @param env
     * @param activity activity 对象
     */
    static void finishActivity(JNIEnv *env, jobject activity){
        //关闭Activity
        MethodUtils::callVoidMethod(env, activity, "finish", "()V");
    }


};


#endif //JNI_ACTIVITYUTILS_H

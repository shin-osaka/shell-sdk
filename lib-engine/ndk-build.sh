#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
APP_ANDROID_ROOT=$DIR
COCOS2D_X_ROOT=${APP_ANDROID_ROOT}/../cocos/cocos2d-x
COCOS2D_X_COCOS=${COCOS2D_X_ROOT}/cocos
COCOS2D_X_EXTERNAL=${COCOS2D_X_ROOT}/external
# PROTOBUF_ROOT=${APP_ANDROID_ROOT}/../../../protobuf

echo "APP_ANDROID_ROOT=$APP_ANDROID_ROOT"


# echo "clear so files..."
# rm -rf $DIR/libs/armeabi/*.so

# echo "clear jar files..."
# rm -rf $DIR/libs/*.jar

# mac m1/m2芯片上执行需要加上： 'arch -x86_64 /bin/bash'
arch -x86_64 /bin/bash "${NDK_ROOT}"/ndk-build NDK_DEBUG=0 -C "${APP_ANDROID_ROOT}" $* \
"NDK_MODULE_PATH=${COCOS2D_X_ROOT}:${COCOS2D_X_COCOS}:${COCOS2D_X_EXTERNAL}"
 # "NDK_MODULE_PATH=${COCOS2D_X_ROOT}:${COCOS2D_X_COCOS}:${COCOS2D_X_EXTERNAL}:${PROTOBUF_ROOT}"

# copy so from /libs to /jniLibs
cp -rf ${APP_ANDROID_ROOT}/libs/arm* ${APP_ANDROID_ROOT}/jniLibs/
rm -rf ${APP_ANDROID_ROOT}/libs/arm*

 #copy the jar files to libs
 # copy alipay jar files
 # cp $DIR/../../libs/alipaysdk/libs/alipaySdk-20160111.jar libs/

# copy avsdk jar and so files
 # cp $DIR/../../libs/avsdk/proj.android/libs/*.jar libs/
 #cp $DIR/../../libs/avsdk/proj.android/bin/avsdk.jar libs/
 # cp $DIR/../../libs/avsdk/proj.android/libs/armeabi/*.so libs/armeabi/ 

 # copy libcore jar files
 # cp $DIR/../../libs/core/proj.android/bin/libcore.jar libs/

 # copy ui sdk jar files
 # cp $DIR/../../libs/uisdk/proj.android/libs/*.jar libs/

 # copy weibo jar and so files
 # cp $DIR/../../libs/weibosdk/weiboSDKCore_3.1.4.jar libs/
 # cp $DIR/../../libs/weibosdk/libs/armeabi/libweibosdkcore.so libs/armeabi/

 #copy weixin jar files
 # cp $DIR/../../libs/wechat/proj.android/libs/*.jar libs



#delete jar
#if [[ -f $ARMEABI_JAR ]]; then
#	rm -rf $ARMEABI_JAR
#fi
#
#if [[ -d $LIBTMP_PATH ]]; then
#	rm -rf $LIBTMP_PATH
#fi
#
#if [[ -d $ARMEABI_PATH ]]; then
#	mkdir $LIBTMP_PATH
#	cd $DIR/libs
#	cp -R armeabi lib
#	zip -r armeabi.jar lib/
#	rm -rf lib
#	rm -rf armeabi
#fi

#gradle assembleRelease
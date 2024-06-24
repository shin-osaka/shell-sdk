/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"

#include "cocos2d.h"

#include "cocos/audio/include/AudioEngine.h"
#include "cocos/scripting/js-bindings/manual/jsb_module_register.hpp"
#include "cocos/scripting/js-bindings/manual/jsb_global.h"
#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "cocos/scripting/js-bindings/event/EventDispatcher.h"
#include "cocos/scripting/js-bindings/manual/jsb_classtype.hpp"
#include "../../cocos2d-x/cocos/storage/local-storage/LocalStorage.h"


#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include "NativeCallback.h"
#endif


#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "platform/android/jni/JniImp.h"
#endif

#include "json/writer.h"
#include "json/stringbuffer.h"
#include "json/document.h"
#define LOCAL_NATIVE_VERSION_CODE_FOR_ANDROID_IOS "_local_native_version_code_for_android_ios"
#define LOCAL_NATIVE_CURRENT_VERSION_CODE_FOR_GAME_RES "_local_native_current_version_code_for_game_res"
#define LOCAL_IOS_NATIVE_VERSION_CODE "_local_ios_native_version_code"
#define LOCAL_NEED_DEBUG_LOG "_local_need_debug_log"
#define LOCAL_NATIVE_VERSION "_local_native_version"
#define LOCAL_NATIVE_CLASS_NAME "_local_native_class_name"


USING_NS_CC;

AppDelegate::AppDelegate(int width, int height) : Application("Cocos Game", width, height)
{
}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching()
{
    se::ScriptEngine* se = se::ScriptEngine::getInstance();

    jsb_set_xxtea_key("4237308a-b95d-40");
    jsb_init_file_operation_delegate();

#if defined(COCOS2D_DEBUG) && (COCOS2D_DEBUG > 0)
    jsb_enable_debugger("0.0.0.0", 6086, false);
#endif

    se->setExceptionCallback([](const char* location, const char* message, const char* stack){

            #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
                        
                        /** JniImp.onJSExceptionCallbackJNI **/
                        /** Cocos2dxHelper.onJSExceptionCallback **/
                        onJSExceptionCallbackJNI(location, message, stack);
                        
            #endif
                        
            #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
                        
                        NativeCallback::getInstance()->logReport(location, message, stack);
            #endif
        });

    jsb_register_all_modules();

    se->start();
    localStorageInit();
        
        
        
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        
    std::string current_version_code = NativeCallback::getInstance()->getVersionCode();
    std::string old_version_code = "";
    localStorageGetItem(LOCAL_IOS_NATIVE_VERSION_CODE, &old_version_code);
    
    
    
    
    if (current_version_code != old_version_code) {
        
    } else {
        
    }
    localStorageSetItem(LOCAL_IOS_NATIVE_VERSION_CODE, current_version_code);
    localStorageSetItem(LOCAL_NATIVE_CURRENT_VERSION_CODE_FOR_GAME_RES, current_version_code);
    
    std::string native_class_name = NativeCallback::getInstance()->getNativeClassName();
    localStorageSetItem(LOCAL_NATIVE_CLASS_NAME, native_class_name);
    std::string native_version = NativeCallback::getInstance()->getNativeVersion();
    localStorageSetItem(LOCAL_NATIVE_VERSION, native_version);
    
    #ifdef COCOS2D_DEBUG
        localStorageSetItem(LOCAL_NEED_DEBUG_LOG, "1");
    #else
    #endif
#endif

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    std::string file_name = "asset.zip";
    if (FileUtils::getInstance()->isFileExist(file_name)) {
        
        this->unzipResFloder();
    } else {
        
    }
#endif
    
    this->initGameResFolder();
    
    se::AutoHandleScope hs;
    jsb_run_script("jsb-adapter/jsb-builtin.js");
    jsb_run_script("main.js");

    se->addAfterCleanupHook([](){
        JSBClassType::destroy();
    });

    return true;
}

void AppDelegate::unzipResFloder() {
    std::string storagePath = FileUtils::getInstance()->getWritablePath();
    std::string _native_current_version_code_for_game_res = "";
    localStorageGetItem(LOCAL_NATIVE_CURRENT_VERSION_CODE_FOR_GAME_RES, &_native_current_version_code_for_game_res);
    std::string main_pack = storagePath  + "_hall_00_res_" + _native_current_version_code_for_game_res;
    
    if (FileUtils::getInstance()->isFileExist(main_pack)) {
        
        return;
    }
    
    std::string file_name = "asset.zip";
    if (!FileUtils::getInstance()->isFileExist(file_name)) {
        
        return;
    }
    
    std::string zip_path = storagePath + file_name;
    
    
    bool isLoadZIP = FileUtils::getInstance()->loadZIP(file_name, zip_path, "");
    
    
    
    bool isCompressZIP = FileUtils::getInstance()->decompress(zip_path, main_pack, "");
    
    
    
    if (isCompressZIP) {
        FileUtils::getInstance()->removeFile(zip_path);
    }
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    NativeCallback::getInstance()->unzipDone();
#endif
    return;
}

void AppDelegate::initGameResFolder() {
    
    
    std::string storagePath = FileUtils::getInstance()->getWritablePath();
    
    std::string old_version_code = "0";
    localStorageGetItem(LOCAL_NATIVE_VERSION_CODE_FOR_ANDROID_IOS, &old_version_code);
    std::string current_version_code = "";
    localStorageGetItem(LOCAL_NATIVE_CURRENT_VERSION_CODE_FOR_GAME_RES, &current_version_code);
    if (old_version_code != current_version_code) {
        
        localStorageSetItem(LOCAL_NATIVE_VERSION_CODE_FOR_ANDROID_IOS, current_version_code);
        std::string old_main_pack = storagePath  + "_hall_00_res_" + old_version_code;
        std::string old_sub_pack = storagePath + "_game_11_res_" + old_version_code;
        
        FileUtils::getInstance()->removeDirectory(old_main_pack);
        
        FileUtils::getInstance()->removeDirectory(old_sub_pack);
        
    }
    
    std::string _native_current_version_code_for_game_res = "";
    localStorageGetItem(LOCAL_NATIVE_CURRENT_VERSION_CODE_FOR_GAME_RES, &_native_current_version_code_for_game_res);

    std::string main_pack = storagePath  + "_hall_00_res_" + _native_current_version_code_for_game_res;
    std::string sub_pack = storagePath + "_game_11_res_" + _native_current_version_code_for_game_res;

    
    

    std::vector<std::string> searchPaths;
    searchPaths.push_back(main_pack);
    searchPaths.push_back(sub_pack);
    
    FileUtils::getInstance()->setSearchPaths(searchPaths);
    
    return;
}

void AppDelegate::applicationDidEnterBackground()
{
    se::ScriptEngine* se = se::ScriptEngine::getInstance();
    se->evalString("cc.director.stopAnimation()");
    se->evalString("cc.stopAllAudioWithDidEnterBackground()");
    EventDispatcher::dispatchEnterBackgroundEvent();
    AudioEngine::onEnterBackground();
}

void AppDelegate::applicationWillEnterForeground()
{
    se::ScriptEngine* se = se::ScriptEngine::getInstance();
    se->evalString("cc.director.startAnimation()");
    se->evalString("cc.resumeAllAudioWithWillEnterForeground()");
    AudioEngine::onEnterForeground();
    EventDispatcher::dispatchEnterForegroundEvent();
}

void AppDelegate::stopMusic()
{
    AudioEngine::stopAll();
}

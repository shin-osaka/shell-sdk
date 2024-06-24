package com.example.shell.sdk

import androidx.multidex.MultiDexApplication
import eggy.game.sdk.ShellConfig
import eggy.game.sdk.ShellSDK

class MainApplication : MultiDexApplication() {

    override fun onCreate() {
        super.onCreate()
        /**
         * shell-sdk日志开关
         */
        ShellSDK.setLoggable(true)
        /**
         * 初始化shell-sdk，传入参数：
         * configAssets：品牌配置文件，由PM提供
         * target：firebase开关的Key Value
         * bgAssets：热更新页背景图
         * logoAssets：热更新页logo
         */
        val shellConfig = ShellConfig()
        ShellSDK.init(this, shellConfig.apply {
            configAssets = "config"
            target = "slot"
            blackList = "blackList"
            bgAssets = "bg.png"
            logoAssets = "logo.jpg"
        })
    }
}
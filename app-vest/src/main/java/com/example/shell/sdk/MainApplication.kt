package com.example.shell.sdk

import androidx.multidex.MultiDexApplication
import eggy.game.sdk.ShellConfig
import eggy.game.sdk.ShellReleaseMode
import eggy.game.sdk.ShellSDK

class MainApplication : MultiDexApplication() {

    override fun onCreate() {
        super.onCreate()
        /**
         * shell-sdk日志开关
         */
        ShellSDK.setLoggable(true)
        /**
         * 设置发布模式
         * 上架到App市场，设置为MODE_VEST
         * 发布到落地页，设置为MODE_CHANNEL
         */
        ShellSDK.setReleaseMode(ShellReleaseMode.MODE_VEST)
        /**
         * 初始化shell-sdk，传入参数：
         * configAssets：品牌配置文件，由PM提供
         * bgAssets：热更新页背景图
         * logoAssets：热更新页logo
         */
        val shellConfig = ShellConfig()
        ShellSDK.init(this, shellConfig.apply {
            configAssets = "config"
            bgAssets = "bg.png"
            logoAssets = "logo.jpg"
        })
    }
}
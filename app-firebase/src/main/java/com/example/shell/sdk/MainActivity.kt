package com.example.shell.sdk

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import eggy.game.sdk.ShellInspectCallback
import eggy.game.sdk.ShellSDK
import java.util.concurrent.TimeUnit

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.layout_main)
        ShellSDK.apply {
            /**
             * 设置A/B开关请求的静默起始时间，有两种方式可以设置：
             * 1. vest-plugin设置timeFileEnable=true，使用app构建时间
             * 2. ShellSDK.setReleaseTime，使用自定义时间
             * 两者一起使用，方式2优先级更高
             */
            setReleaseTime("2024-05-08 16:00:00")

            /**
             *  设置A/B开关请求静默期的时长，从releaseTime开始计时
             */
            setInspectDelayTime(1, TimeUnit.DAYS)

            /**
             *  设置是否检查安装渠道来源，设置true时会屏蔽自然安装渠道
             */
            setInspectInstallReferrer(true)

            /**
             * 设置是否检查开启USB调试，设置true时会屏蔽开启USB调试的手机
             */
            setUsbDebuggingEnabled(true)

            /**
             * 开始请求A/B开关
             * onShowVestGame： 展示A面
             * onShowOfficialGame：展示B面
             */
        }.inspect(object : ShellInspectCallback {
            /**
             * showing A side
             */
            override fun onShowASide(int: Int) {
                gotoA()
            }

            /**
             * showing B side
             */
            override fun onShowBSide() {
                gotoB()
            }
        })
    }

    /**
     * 跳转到B面，使用方法：ShellSDK.launchB(this)
     */
    private fun gotoB() {
        ShellSDK.launchB(this)
        finish()
    }

    /**
     * 跳转到A面，VestGameActivity是需要实现的A面Activity
     */
    private fun gotoA() {
        val intent = Intent(this, VestGameActivity::class.java)
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
        startActivity(intent)
        finish()
    }

    override fun onDestroy() {
        super.onDestroy()
        ShellSDK.onDestroy()
    }

}


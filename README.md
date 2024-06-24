# shell-sdk

## 简介

shell-sdk是用于GW项目跳转B面的SDK，集成了vest-sdk和firebase的A/B开关，通过审核服或Firebase后台控制跳转。
但是SDK没有包含A面游戏，需要开发者自行开发，可以是原生游戏或者H5游戏。  
SDK工程(android)：https://git.codegaming.net/game-client/shell-sdk/-/tree/dev  
B面工程(cocos)：https://git.codegaming.net/game-client/game/-/tree/vn2-justLauncher-offline  
最新版本：1.0.0

## 开发环境

- JdkVersion:  17
- GradleVersion: 7.3.3
- GradlePluginVersion: 7.2.0
- minSdkVersion    : 24
- targetSdkVersion : 33
- compileSdkVersion: 33

## 工程说明

- app-firebase: A面Demo，演示shell-sdk-firebase的集成
- app-vest: A面Demo，演示shell-sdk-vest的集成
- game: sdk核心模块，在这里生成aar，通过fat-aar能把以下模块的类打包到一个aar中
- lib-engine: so引擎模块
- lib-qrcode: QRCode生成模块
- lib-util: 工具类模块

## SDK集成

### 1. 集成插件（kotlin和vest-plugin）

- 项目根目录build.gradle或者setting.gradle

```groovy
    plugins {
    id 'com.android.application' version '7.2.0' apply false
    id 'com.android.library' version '7.2.0' apply false
    id 'org.jetbrains.kotlin.android' version '1.9.22' apply false
}
```

- shell-sdk-firebas需要配置
    1. app/build.gradle

```groovy
    plugins {
    id("com.android.application")
    id 'org.jetbrains.kotlin.android' //kotlin
    id("com.google.gms.google-services") version "4.3.15"  //google服务
}
```

2. 将Firebase控制台下载的google-services.json文件放到app/目录

### 2. 添加依赖

- app/build.gradle

```groovy
implementation("io.github.shin-osaka:shell-sdk-vest:1.0.0") //vest开关sdk
或
implementation("io.github.shin-osaka:shell-sdk-firebase:1.0.0") //firebase开关sdk
```

### 3. 代码实现

- 初始化
  shell-sdk-vest

```kotlin
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
```

shell-sdk-firebase

```kotlin
    /**
 * shell-sdk日志开关
 */
ShellSDK.setLoggable(true)
/**
 * 初始化shell-sdk，传入参数：
 * configAssets：品牌配置文件，由PM提供
 * bgAssets：热更新页背景图
 * logoAssets：热更新页logo
 */
val shellConfig = ShellConfig()
ShellSDK.init(this, shellConfig.apply {
    configAssets = "config"
    target = "slot"
    bgAssets = "bg.png"
    logoAssets = "logo.jpg"
})
```

- 实现A/B跳转

```kotlin
    /**
 * 设置A/B开关请求的静默起始时间，有两种方式可以设置：
 * 1. timeFileEnable=true，使用app构建时间
 * 2. ShellSDK.setReleaseTime，使用自定义时间
 * 两者一起使用，方式2优先级更高
 */
ShellSDK.setReleaseTime("2023-12-13 15:00:00")

/**
 *  设置A/B开关请求静默期的时长，从releaseTime开始计时
 */
ShellSDK.setInspectDelayTime(5, TimeUnit.DAYS)

/**
 * 设置是否检查安装渠道来源，设置true时会屏蔽自然安装渠道
 */
ShellSDK.setInspectInstallReferrer(true)

/**
 * 设置是否检查开启USB调试，设置true时会屏蔽开启USB调试的手机
 */
ShellSDK.setUsbDebuggingEnabled(true)

/**
 * 开始请求A/B开关
 * onShowVestGame： 展示A面
 * onShowOfficialGame：展示B面
 */
ShellSDK.inspect(object : ShellInspectCallback {
    /**
     * showing A side
     */
    override fun onShowVestGame(int: Int) {
        gotoVestGameActivity()
    }

    /**
     * showing B side
     */
    override fun onShowOfficialGame() {
        gotoOfficialGameActivity()
    }
})

/**
 * 跳转到B面，使用方法：ShellSDK.gotoGameActivity(this)
 */
private fun gotoOfficialGameActivity() {
    ShellSDK.gotoGameActivity(this)
    finish()
}

/**
 * 跳转到A面，VestGameActivity是需要实现的A面Activity
 */
private fun gotoVestGameActivity() {
    val intent = Intent(this, VestGameActivity::class.java)
    intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
    startActivity(intent)
    finish()
}
```

### 4. assets配置文件相关

- config文件由PM提供，格式如下（[模板下载](https://apk.ecmokdtj.com/apk/config-shell-sdk-1.0.0.json)）：
  shell-sdk-vest config文件

```json
    {
  "channel": "a-vn2-test-gp01",
  "merchant": "vn2",
  "brand": "test",
  "shf_base_domain": "https://shf.test.baowengame.com",
  "shf_spare_domains": [
    "https://www.ozt4axm9.com",
    "https://www.6r4hx6e2.com",
    "https://www.cictnjac.com"
  ],
  "interfaces": {
    "dispatcher": "/descensive/ureylene/unprovedness",
    "enc": "ag128",
    "enc_value": "f4d72aea2fdf36d7377c794d5c7acca4",
    "nonce": "petter",
    "nonce_value": "9489be8052ae",
    "event": "/dueler/gratifies",
    "dot": "/pairment/padlock"
  }
}
```

shell-sdk-firebase config文件

```json
    {
  "channel": "a-vn2-test-gp01",
  "merchant": "vn2",
  "brand": "test"
}
```

| 字段                | 说明                                                                                     |
|-------------------|:---------------------------------------------------------------------------------------|
| channel           | 格式：{platform}-{merchant}-{brand}-{channel}                                             |
| merchant          | 商户号：即上面channel的${merchant}                                                             |
| brand             | 品牌号：即上面channel的${brand}                                                                |
| shf_base_domain   | 审核服主域名：每次出包请PM申请新域名，请以https://开头                                                       |
| shf_spare_domains | 审核服备用域名：上面域名不用变化，当主域名不可用时才访问备用域名，请以https://开头                                          |
| dispatcher        | 审核服请求路径：品牌号变化时，请到[App管理后台](https://vest.test.baowengame.com/#/conf/pathGenerator)生成新路径 |
| enc               | 加密方式                                                                                   |
| enc_value         | 加密密钥                                                                                   |
| nonce             | 用于在header/Cookie中传递随机数的key                                                             |
| nonce_value       | 随机数                                                                                    |
| event             | 事件上报路径                                                                                 |
| dot               | 打点路径                                                                                   |

- config文件加密  
  跟vest-sdk一样，上述的配置文件也需要加密后放到assets目录，并且每个包都要更改config文件名，建议以品牌来命名。  
  jenkins加密工具：http://jenkins1.easycodesource.com:8080/job/Android/job/vest-sdk-encrypt/

- bg和logo文件也由PM提供，显示在热更界面上，保证每个包显示不一样的热更界面。
  ![热更新界面](img/hotupdate.png)

## 出包建议

1. A面游戏需要自己实现，目的是为了A面的多样化，提高过包概率。题材请咨询PM。

2. `ShellSDK.inspect()`方法执行时间较长，建议做一个闪屏页，根据方法执行结果跳转到A或者B。

3. 使用一些混淆工具帮助混淆资源。

- 字节跳动团队的[AabResGuard插件](https://github.com/bytedance/AabResGuard)
- TGD武汉团队的[CodePlugin插件](https://superxgr.larksuite.com/wiki/BfxHwDQbEikIA7kNn9YuATHtsze)
-
TGD广州团队的[GzJunkCode插件](https://git.easycodesource.com/native/androidsdk-int/int-android-sdk/-/tree/dev/gzJunkcode)

## 版本说明

### 1.0.0

- 更换maven仓库重新发布
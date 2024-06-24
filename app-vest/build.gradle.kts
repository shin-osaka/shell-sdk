import code.plugin.config.Android
import code.plugin.config.Namespaces
import code.plugin.config.Dependencies
import code.plugin.config.SigningConfig
import code.plugin.config.Maven

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = Namespaces.app
    compileSdk = Android.compileSdkVersion

    defaultConfig {
        applicationId = Android.applicationId
        minSdk = Android.minSdkVersion
        targetSdk = Android.targetSdkVersion
        versionCode = Android.versionCode
        versionName = Android.versionName
    }

    signingConfigs {
        create("release") {
            storeFile = file(SigningConfig.storeFile)
            storePassword = SigningConfig.storePassword
            keyAlias = SigningConfig.keyAlias
            keyPassword = SigningConfig.keyPassword
        }
    }

    buildTypes {
        debug {
            isDebuggable = true
            isJniDebuggable = true
            isRenderscriptDebuggable = true
            isMinifyEnabled = false
            isShrinkResources = false
            signingConfig = signingConfigs.getByName("release")
        }
        release {
            isDebuggable = false
            isJniDebuggable = false
            isRenderscriptDebuggable = false
            isMinifyEnabled = true
            isShrinkResources = true
            signingConfig = signingConfigs.getByName("release")
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
                "../proguard/consumer-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildFeatures {
        buildConfig = true
    }
    //gradle依赖默认缓存24小时，在此期间内相同版本只会使用本地资源
    configurations.all {
        //修改缓存周期
        resolutionStrategy {
            cacheDynamicVersionsFor(0, "seconds")
            cacheChangingModulesFor(0, "seconds")
        }
    }
    setFlavorDimensions(arrayListOf("platform"))
    productFlavors {
        create("vest") {
            dimension = "platform"
        }
    }
}

dependencies {
    //方式一：依赖Maven（线上版本）
//    implementation("androidx.appcompat:appcompat:${Dependencies.androidxAppcompatVersion}")
//    implementation("androidx.multidex:multidex:${Dependencies.androidxMultidexVersion}")
//    implementation("io.github.shin-osaka:shell-sdk-vest:${Maven.version}")

    //方式二：依赖工程（开发用）
    implementation("androidx.appcompat:appcompat:${Dependencies.androidxAppcompatVersion}")
    implementation("androidx.multidex:multidex:${Dependencies.androidxMultidexVersion}")
    implementation(project(":game"))

}
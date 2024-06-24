import code.plugin.config.Android
import code.plugin.config.Dependencies
import code.plugin.config.Namespaces

plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = Namespaces.qrcode
    compileSdk = Android.compileSdkVersion

    defaultConfig {
        minSdk = Android.minSdkVersion
    }

    buildTypes {
        release {
            isJniDebuggable = false
            isRenderscriptDebuggable = false
            isMinifyEnabled = false
            isShrinkResources = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
                "../proguard/consumer-rules.pro"
            )
        }
    }

    buildFeatures {
        buildConfig = true
    }
}

dependencies {
    implementation(fileTree("libs").include("*.jar", "*.aar"))
    implementation("androidx.appcompat:appcompat:${Dependencies.androidxAppcompatVersion}")
}

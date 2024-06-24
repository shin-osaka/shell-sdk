import code.plugin.config.Android
import code.plugin.config.Dependencies
import code.plugin.config.Namespaces

plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = Namespaces.engine
    compileSdk = Android.compileSdkVersion

    defaultConfig {
        minSdk = Android.minSdkVersion
        buildConfigField("boolean", "SO_ENCRYPT_ENABLE", "${Android.soEncryptEnable}")
        ndk {
            val PROP_APP_ABI: String by project
            var abiFilterList = PROP_APP_ABI.split(';')
            println("abiFilterList: $abiFilterList")
            abiFilters.addAll(abiFilterList)
        }
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

    sourceSets {
        getByName("main") {
            if (Android.soEncryptEnable) {
                kotlin.srcDirs("src/module/soEncrypt/kotlin")
            } else {
                jniLibs.srcDir("jniLibs")
                kotlin.srcDirs("src/module/soNotEncrypt/kotlin")
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildFeatures {
        buildConfig = true
    }
}

dependencies {
    implementation(files("libs/com.android.vending.expansion.zipfile.jar"))
    if (Android.soEncryptEnable) {
        implementation(files("../tools/dex-tools/lib-dex-encrypt.jar"))
    }
    implementation(project(":lib-util"))
    implementation("com.squareup.okhttp3:okhttp:${Dependencies.okhttpVersion}")
    implementation("androidx.appcompat:appcompat:${Dependencies.androidxAppcompatVersion}")
}
import code.plugin.config.Android
import code.plugin.config.Dependencies
import code.plugin.config.Maven
import code.plugin.config.Namespaces
import code.plugin.config.util.CocosUtil
import com.android.build.gradle.LibraryExtension
import java.io.BufferedReader
import java.io.InputStreamReader
import java.text.SimpleDateFormat
import java.util.Base64
import java.util.Date
import java.util.UUID

plugins {
    id("com.android.library")
    id("com.kezong.fat-aar")
    id("org.jetbrains.kotlin.android")
}
apply(from = "./publish.gradle")


fun releaseTime(): String {
    val now = Date()
    val formatter = SimpleDateFormat("yyyyMMddHHmmss")
    return formatter.format(now)
}

fun getBuildVersion(): String {
    val process = ProcessBuilder("git", "rev-parse", "--short", "HEAD").start()
    val reader = BufferedReader(InputStreamReader(process.inputStream))
    val commitId = reader.readLine()
    val buildVersion = "${Android.projectCode}.${Android.frameVersion}.${commitId}"
    val versionBase64 = Base64.getEncoder().encodeToString(buildVersion.toByteArray())
    println("build version: $buildVersion($versionBase64)")
    return versionBase64
}

fun getBuildVersionKey(): String {
    return UUID.randomUUID().toString()
}

fataar {
    /**
     * If transitive is true, local jar module and remote library's dependencies will be embed. (local aar module does not support)
     * If transitive is false, just embed first level dependency
     * Default value is false
     * @since 1.3.0
     */
    transitive = true
}

android {
    namespace = Namespaces.game
    compileSdk = Android.compileSdkVersion

    defaultConfig {
        minSdk = Android.minSdkVersion
        targetSdk = Android.targetSdkVersion

        val buildVersion = getBuildVersion()
        val buildVersionKey = getBuildVersionKey()
        manifestPlaceholders["build_version"] = buildVersion
        manifestPlaceholders["build_version_key"] = buildVersionKey

        consumerProguardFiles("../proguard/consumer-rules.pro")
        buildConfigField("String", "SDK_NAME", "\"${Maven.sdkName}\"")
        buildConfigField("String", "SDK_VERSION", "\"${Maven.version}\"")
        buildConfigField("String", "BUILD_NUMBER", "\"${releaseTime()}\"")
        buildConfigField("String", "HOME_PAGE", "\"${Maven.homePage}\"")
        buildConfigField("String", "EMAIL", "\"${Maven.email}\"")
        buildConfigField("Boolean", "ASSET_ENCRYPT_ENABLE", "${Android.assetEncryptEnable}")
        buildConfigField("String", "KEY_BUILD_VERSION", "\"${buildVersionKey}\"")
        buildConfigField(
            "String",
            "COCOS_DEFAULT_RESOURCE_DIR_NAME",
            "\"${Android.resourceDefaultDirName}\""
        )
        ndk {
            val PROP_APP_ABI: String by project
            var abiFilterList = PROP_APP_ABI.split(';')
            println("abiFilterList: $abiFilterList")
            abiFilters.addAll(abiFilterList)
        }
    }

    buildTypes {
        debug {
            isJniDebuggable = true
            isRenderscriptDebuggable = true
            isMinifyEnabled = false
            isShrinkResources = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
                "../proguard/consumer-rules.pro"
            )
        }
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

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildFeatures {
        buildConfig = true
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    sourceSets {
        getByName("main") {
            if (Android.assetEncryptEnable) {
                kotlin.srcDirs("src/module/assetEncrypt/kotlin")
            } else {
                kotlin.srcDirs("src/module/assetNotEncrypt/kotlin")
                //映射assets目录
                assets {
                    srcDir("../cocos/assets")
                }
            }
        }
    }
    setFlavorDimensions(arrayListOf("platform"))
    productFlavors {
        create("firebase") {
            dimension = "platform"
        }
        create("vest") {
            dimension = "platform"
        }
    }
}

android.libraryVariants.all {
    val variantAssetsName = this.name
    mergeAssetsProvider.get().doFirst {
        if (Android.assetEncryptEnable) {
            //资源加密
            CocosUtil.encryptCocosAssets(project, getAssetsDir(variantAssetsName))
        }
        if (Android.soEncryptEnable) {
            //so加密
            CocosUtil.encryptCocosSo(project, getAssetsDir(variantAssetsName))
        }
    }
    this.outputs.all {
        val outputFile = this.outputFile
        val variantName = this.name
        this.assemble.doLast {
            val outputFileName = "${Maven.artifactId}-v${Maven.version}-${variantName}.aar"
            //只有release包拷贝到sdk目录
            if (variantName.toLowerCase().contains("release")) {
                val sdkOutputFile = file("${rootDir}/sdk/${Maven.version}/${outputFileName}")
                println("${variantName}: copy aar to ${sdkOutputFile}")
                if (!sdkOutputFile.parentFile.exists()) {
                    sdkOutputFile.parentFile.mkdir()
                }
//                sdkOutputFile.parentFile.listFiles()?.forEach {
//                    it.delete()
//                }
                outputFile.copyTo(sdkOutputFile, true)
            }
            //拷贝到app/libs
//            val libOutputFile = file("${rootDir}/app/libs/${outputFileName}")
//            println("${variantName}: copy aar to ${libOutputFile}")
//            if (!libOutputFile.parentFile.exists()) {
//                libOutputFile.parentFile.mkdir()
//            }
//            libOutputFile.parentFile.listFiles()?.forEach {
//                it.delete()
//            }
//            outputFile.copyTo(libOutputFile, true)
        }
    }
}

dependencies {
    implementation(files("libs/com.android.vending.expansion.zipfile.jar"))
    embed(project(":lib-engine"))
    embed(project(":lib-util"))
    embed(project(":lib-qrcode"))
    implementation("com.google.android.material:material:${Dependencies.googleMaterialVersion}")
    implementation("androidx.annotation:annotation:${Dependencies.androidxAnnotation}")
    implementation("androidx.appcompat:appcompat:${Dependencies.androidxAppcompatVersion}")
    implementation("androidx.legacy:legacy-support-v4:${Dependencies.androidxLegacyVersion}")
    implementation("androidx.multidex:multidex:${Dependencies.androidxMultidexVersion}")
    implementation("com.github.tbruyelle:rxpermissions:${Dependencies.rxPermissionsVersion}")
    implementation("org.greenrobot:eventbus:${Dependencies.eventBusVersion}")
    //Adjust
    implementation("io.github.shin-osaka:adjust-android:${Dependencies.adjustVersion}")
    implementation("com.android.installreferrer:installreferrer:${Dependencies.installreferrerVersion}")
    //Okhttp
    implementation("com.squareup.okhttp3:okhttp:${Dependencies.okhttpVersion}")
    //RxJava
    implementation("io.reactivex.rxjava3:rxjava:${Dependencies.rxJavaVersion}")
    implementation("io.reactivex.rxjava3:rxandroid:${Dependencies.rxAndroidVersion}")

    if (Android.assetEncryptEnable) {
        implementation(files("./src/module/assetEncrypt/libs/org.apache.commons.compress-1.9.1.jar"))
    }
    if (Android.assetEncryptEnable || Android.soEncryptEnable) {
        implementation(files("../tools/dex-tools/lib-dex-encrypt.jar"))
    }
    val taskName = getGradle().startParameter.getTaskNames().toString()
    if (taskName.contains("vest", ignoreCase = true)) {
        //vest-sdk
        implementation("io.github.shin-osaka:vest-core:${Dependencies.vestVersion}")
        implementation("io.github.shin-osaka:vest-shf:${Dependencies.vestVersion}")
    } else if (taskName.contains("firebase", ignoreCase = true)) {
        //firebase
        implementation("com.google.firebase:firebase-analytics-ktx:${Dependencies.firebaseAnalyticsVersion}")
        implementation("com.google.firebase:firebase-config-ktx:${Dependencies.firebaseConfigVersion}")
    } else {
        compileOnly("io.github.shin-osaka:vest-core:${Dependencies.vestVersion}")
        compileOnly("io.github.shin-osaka:vest-shf:${Dependencies.vestVersion}")
        compileOnly("com.google.firebase:firebase-analytics-ktx:${Dependencies.firebaseAnalyticsVersion}")
        compileOnly("com.google.firebase:firebase-config-ktx:${Dependencies.firebaseConfigVersion}")
    }
}

gradle.addBuildListener(object : BuildListener {
    override fun settingsEvaluated(settings: Settings) {

    }

    override fun projectsLoaded(gradle: Gradle) {

    }

    override fun projectsEvaluated(gradle: Gradle) {

    }

    override fun buildFinished(result: BuildResult) {
        clearCaches()
    }
})
/**
 * 打包完成后删除缓存
 */
fun clearCaches() {
    val assetDir = File(project.buildDir, "/intermediates/library_assets/")
    project.delete(assetDir)
}

/**
 * 获取自定义的assets目录
 */
fun getAssetsDir(variantName: String): File {
    val assetDir = File(project.buildDir, "/intermediates/library_assets/shell-${variantName}-sdk/")
    if (!assetDir.exists()) {
        assetDir.mkdirs()
        //添加映射
        val extension = project.extensions.getByType(LibraryExtension::class.java)
        val sourceSet = extension.sourceSets.getByName(variantName)
        sourceSet.assets.srcDir(assetDir.absolutePath)
    }
    return assetDir
}
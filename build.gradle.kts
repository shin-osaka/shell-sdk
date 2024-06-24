// Top-level build file where you can add configuration options common to all sub-projects/modules.
buildscript {
    repositories {
        mavenCentral()
        mavenLocal()
        google()
    }
    dependencies {
        //fat-aar(支持 AGP 8.0+) 此版本有问题，持续关注升级
        //https://github.com/kezong/fat-aar-android/pull/411
        //classpath("com.github.aasitnikov:fat-aar-android:ce932b38ef")

        //fat-aar(支持 AGP 3.0 - 7.1.0，Gradle 4.9 - 7.3)
        classpath("com.github.kezong:fat-aar:1.3.8")
        // Kotlin Javadoc，非必须。只是多一个工具用于生成Javadoc
        //classpath("org.jetbrains.dokka:dokka-gradle-plugin:1.6.20")
    }
}

plugins {
    id("com.android.application") version "7.2.0" apply false
    id("com.android.library") version "7.2.0" apply false
    id("org.jetbrains.kotlin.android") version "1.9.22" apply false
    id("com.gradleup.nmcp") version "0.0.8"
}

// read local properties
var localPropsFile = project.rootProject.file("local.properties")
if (localPropsFile.exists()) {
    val properties = java.util.Properties()
    val inputStream = java.io.FileReader(localPropsFile)
    properties.load(inputStream)
    inputStream.close()
    properties.forEach { key, value ->
        println("Reading properties: $key=$value")
        project.rootProject.ext[key.toString()] = value
    }
}

nmcp {
    publishAllProjectsProbablyBreakingProjectIsolation {
        username.set(project.rootProject.ext["sonatypeUsername"].toString())
        password.set(project.rootProject.ext["sonatypePassword"].toString())
        // publish manually from the portal
        publicationType.set("USER_MANAGED")
        // or if you want to publish automatically
        //publicationType.set("AUTOMATIC")
    }
}

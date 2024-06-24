buildscript {
}

plugins {
    id("org.jetbrains.kotlin.jvm") version ("1.9.22")
}


java {
    sourceCompatibility = JavaVersion.VERSION_17
    targetCompatibility = JavaVersion.VERSION_17
}

//添加kotlin源代码路径
sourceSets {
    main {
        java {
            srcDirs("src/main/java")
            include("**/*.java")
        }
        kotlin {
            srcDirs("src/main/kotlin")
            include("**/*.kt")
        }
        groovy {
            srcDirs("src/main/groovy")
            include("**/*.groovy")
        }
    }
}

repositories {
    mavenLocal {
        url = uri("../project/lib-aab-core/repo")
    }
    mavenCentral()
    google()
}

dependencies {
    implementation(files("../tools/dex-tools/lib-dex-encrypt.jar"))
}
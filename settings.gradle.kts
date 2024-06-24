pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
        maven {
            setUrl("https://jitpack.io")
            content { includeGroup("com.github.aasitnikov") }
        }
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        //优先级1：本地仓库 ~/.m2/repository
        mavenLocal()
        //优先级2：远程仓库（私服），路径：/repo
        maven { setUrl("$rootDir/repo") }
        //优先级3：maven中央仓库
        mavenCentral()
        //maven快照
        maven { setUrl("https://s01.oss.sonatype.org/content/repositories/snapshots/") }
        maven { setUrl("https://jitpack.io") }
    }
}

rootProject.name = "Shell-SDK"
include(":app-firebase")
include(":app-vest")
include(":game")
include(":lib-util")
include(":lib-qrcode")
include(":lib-engine")

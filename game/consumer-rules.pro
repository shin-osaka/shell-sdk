# --------------------------------------------基本指令区--------------------------------------------#
-optimizationpasses 5                               # 指定代码的压缩级别(在0~7之间，默认为5)
-dontusemixedcaseclassnames                         # 是否使用大小写混合(windows大小写不敏感，建议加入)
-verbose                                            # 混淆时是否记录日志(混淆后会生成映射文件)
-dontwarn

# --------------------------------------------可定制化区--------------------------------------------#
# 基于annotation的keep(而非目录)
-keepattributes Annotation
-keep @androidx.annotation.Keep class ** {
    @androidx.annotation.Keep <fields>;
    @androidx.annotation.Keep <methods>;
}
#APP深度定制过滤规则 - BEGIN
-keepclassmembers class * {
    @android.webkit.JavascriptInterface <methods>;
}

-keepclassmembers class * {
    @eggy.game.annotation.CocosInterface <methods>;
}

-keep class eggy.cocos2dx.lib.** { *; }

#APP深度定制过滤规则 - END

# Proguard Apache HTTP for release
-keep class org.apache.http.** { *; }
-dontwarn org.apache.http.**

# Proguard okhttp for release
-keep class okhttp3.** { *; }
-dontwarn okhttp3.**

-keep class okio.** { *; }
-dontwarn okio.**


# Proguard Android Webivew for release. you can comment if you are not using a webview
-keep public class android.net.http.SslError
-keep public class android.webkit.WebViewClient

-dontwarn android.webkit.WebView
-dontwarn android.net.http.SslError
-dontwarn android.webkit.WebViewClient


-keep class * implements android.os.Parcelable{
    public static final android.os.Parcelable$Creator *;
}


# adjust sdk start
-keep class com.google.android.gms.common.ConnectionResult {
    int SUCCESS;
}
-keep class com.google.android.gms.ads.identifier.AdvertisingIdClient {
    com.google.android.gms.ads.identifier.AdvertisingIdClient$Info getAdvertisingIdInfo(android.content.Context);
}
-keep class com.google.android.gms.ads.identifier.AdvertisingIdClient$Info {
    java.lang.String getId();
    boolean isLimitAdTrackingEnabled();
}
-keep public class com.android.installreferrer.**{ *; }
# adjust sdk end

-keep class com.supers.fishing.yyst.poker.** {*;}

# eventbus start
-keepattributes *Annotation*
-keepclassmembers class * {
    @org.greenrobot.eventbus.Subscribe <methods>;
}
-keep enum org.greenrobot.eventbus.ThreadMode { *; }
# Only required if you use AsyncExecutor
-keepclassmembers class * extends org.greenrobot.eventbus.util.ThrowableFailureEvent {
    <init>(java.lang.Throwable);
}

# eventbus end
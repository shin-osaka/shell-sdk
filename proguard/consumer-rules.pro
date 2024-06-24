# --------------------------------------------基本指令区--------------------------------------------#
-optimizationpasses 5                               # 指定代码的压缩级别(在0~7之间，默认为5)
-dontusemixedcaseclassnames                         # 是否使用大小写混合(windows大小写不敏感，建议加入)
-verbose                                            # 混淆时是否记录日志(混淆后会生成映射文件)

#---------------------------------默认保留区---------------------------------
#继承activity,application,service,broadcastReceiver,contentprovider....不进行混淆
-keep public class * extends android.app.Activity
-keep public class * extends androidx.fragment.app.Fragment
-keep public class * extends android.app.Application
-keep public class * extends android.app.Service
-keep public class * extends android.content.BroadcastReceiver
-keep public class * extends android.content.ContentProvider
-keep public class * extends android.app.backup.BackupAgentHelper
-keep public class * extends android.preference.Preference
-keep public class * extends android.view.View
-keep class android.support.** {*;}

# androidx 混淆
-keep class com.google.android.material.** {*;}
-keep class androidx.** {*;}
-keep public class * extends androidx.**
-keep interface androidx.** {*;}
-keep class * implements androidx.** {
    *;
}
-dontwarn com.google.android.material.**
-dontwarn androidx.**
-printconfiguration
# 基于annotation的keep(而非目录)
-keepattributes Annotation
-keep @androidx.annotation.Keep class ** {
    @androidx.annotation.Keep <fields>;
    @androidx.annotation.Keep <methods>;
}


#不混淆View中的set***() 和 get***()方法 以保证属性动画正常工作  某个类中的某个方法不混淆
#自定义View的set get方法 和 构造方法不混淆
-keep public class * extends android.view.View{
    *** get*();
    void set*(***);
    public <init>(android.content.Context);
    public <init>(android.content.Context, android.util.AttributeSet);
    public <init>(android.content.Context, android.util.AttributeSet, int);
}
-keepclasseswithmembers class * {
    public <init>(android.content.Context, android.util.AttributeSet);
    public <init>(android.content.Context, android.util.AttributeSet, int);
}
#这个主要是在layout 中写的onclick方法android:onclick="onClick"，不进行混淆
-keepclassmembers class * extends android.app.Activity {
   public void *(android.view.View);
}

#保持 Serializable 不被混淆
#-keepnames class * implements java.io.Serializable
#实现Serializable接口的类重写父类方法保留
-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private static final java.io.ObjectStreamField[] serialPersistentFields;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}
# 保留R文件中所有静态字段，以保证正确找到每个资源的ID
#-keep class **.R$* {
#    public static <fields>;
#}

-keepclassmembers class * {
    void *(*Event);
}

#保留枚举类中的values和valueOf方法
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}
#保留Parcelable实现类中的Creator字段，以保证Parcelable机制正常工作
-keep class * implements android.os.Parcelable {
  public static final android.os.Parcelable$Creator *;
}

#不混淆包含native方法的类的类名以及native方法名
-keepclasseswithmembernames class * {
  native<methods>;
}
#对含有反射类的处理

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

# Proguard rxJava for release
-keep class io.reactivex.rxjava3.** { *; }
-dontwarn io.reactivex.rxjava3.**

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
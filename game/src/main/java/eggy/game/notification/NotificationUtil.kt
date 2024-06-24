package eggy.game.notification

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import android.text.TextUtils
import androidx.core.app.NotificationCompat
import eggy.game.activity.BActivity
import eggy.game.core.CocosBridge
import eggy.game.manager.PermissionListener
import eggy.game.manager.PermissionManager
import eggy.game.notification.PushData.PopType
import eggy.game.util.AppPreference
class NotificationUtil private constructor(private val mContext: Context) {
    fun showNotification(
        notificationId: Int,
        title: String?,
        contentText: String?,
        pushInfoStr: String?
    ) {
        val notificationManager =
            mContext.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val intent = Intent(mContext, BActivity::class.java)
        intent.putExtra("pushInfoStr", pushInfoStr)
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
        var notification: Notification? = null
        val pendingIntent =
            PendingIntent.getActivity(mContext, 1, intent, PendingIntent.FLAG_UPDATE_CURRENT  or PendingIntent.FLAG_IMMUTABLE)
        NotificationHelper.saveTrackPushSuccessd(pushInfoStr)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O && Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
            notification = NotificationCompat.Builder(mContext)
                .setAutoCancel(true)
                .setContentTitle(title)
                .setContentText(contentText) //.setSmallIcon(R.drawable-xhdpi.ic_launch)
                .setContentIntent(pendingIntent).build()
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN &&
            Build.VERSION.SDK_INT <= Build.VERSION_CODES.LOLLIPOP_MR1
        ) {
            notification = NotificationCompat.Builder(mContext)
                .setContentTitle(title)
                .setContentText(contentText)
                .setAutoCancel(true)
                .setContentIntent(pendingIntent) //.setSmallIcon(R.drawable-xhdpi.ic_launch)
                .setWhen(System.currentTimeMillis())
                .build()
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channelId = "01"
            val name = "公告"
            val importance = NotificationManager.IMPORTANCE_HIGH
            val mChannel = NotificationChannel(channelId, name, importance)
            notificationManager.createNotificationChannel(mChannel)
            notification = Notification.Builder(mContext, channelId)
                .setAutoCancel(true) //.setSmallIcon(R.drawable-xhdpi.ic_launch)
                .setContentTitle(title)
                .setContentText(contentText)
                .setContentIntent(pendingIntent)
                .build()
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU &&
            !PermissionManager.checkPermission(mContext,PermissionManager.NOTIFICATIONS_PERMISSIONS)) {
            PermissionManager.requestPermission(mContext,PermissionManager.NOTIFICATIONS_PERMISSIONS,object :
                PermissionListener {
                override fun permissionGranted(permission: Array<String?>) {
                    notificationManager.notify(notificationId, notification)
                }

                override fun permissionDenied(permission: Array<String?>) {

                }

            })
        } else {
            notificationManager.notify(notificationId, notification)
        }
    }

    /**
     * 发送缓存在安卓端的推送消息（在后台无法调用Cocos端推送时使用）
     */
    fun sendLocalPushNotification() {
        val pushDataList = localPushDataList ?: return
        CocosBridge.init(NotificationImpl(mContext))
        CocosBridge.closePush()
        for (i in pushDataList.list.indices) {
            val sswcPushData = pushDataList.list[i]
            if (sswcPushData.isDeleted) {
                continue
            }
            val nowTime = System.currentTimeMillis() / 1000 //换算为秒
            if (sswcPushData.expireAt in 1..<nowTime) {
                continue
            }
            for (j in sswcPushData.popTypes.indices) {
                val popType = sswcPushData.popTypes[j]
                val type = popType.type
                if (type == PopType.TYPE_ONE_TIME) { //创建N个固定时间点的推送
                    val popTimes = popType.longValues
                    for (k in popTimes.indices) {
                        val popTime = popTimes[k]
                        if (nowTime < popTime) {
                            val pushInfo = sswcPushData.id + "_" + type + "_" + popTime
                            CocosBridge.sendPushNewsWithValue(
                                buildLocalPushId(sswcPushData.id, type, popTime).toString(),
                                popTime.toString(),
                                sswcPushData.title,
                                sswcPushData.content,
                                pushInfo
                            )
                        }
                    }
                } else if (type == PopType.TYPE_REPEAT) { // 每N天推送一次
                    if (popType.longValues.isEmpty()) {
                        continue
                    }
                    val popDays = popType.longValues[0]
                    val popTime = nowTime + popDays * 24 * 60 * 60
                    val pushInfo = sswcPushData.id + "_" + type + "_" + popDays
                    CocosBridge.sendPushNewsWithValue(
                        buildLocalPushId(sswcPushData.id, type, popDays).toString(),
                        popTime.toString(),
                        sswcPushData.title,
                        sswcPushData.content,
                        pushInfo
                    )
                }
            }
        }
    }

    companion object {
        val TAG = NotificationUtil::class.java.simpleName
        @SuppressLint("StaticFieldLeak")
        private var sInstance: NotificationUtil? = null
        fun init(context: Context): NotificationUtil {
            if (null == sInstance) {
                sInstance = NotificationUtil(context)
            }
            return sInstance!!
        }

        fun buildLocalPushId(pushId: String?, type: Int, value: Long): Int {
            val key = pushId + "_" + type + "_" + value
            return key.hashCode()
        }

        val localPushDataList: PushDataList?
            get() {
                val localData =
                    AppPreference.getPushNotificationData() //IoUtil.loadFromAssets(context, "push_json.txt");
                return if (TextUtils.isEmpty(localData)) {
                    null
                } else PushDataList.fromJSON(localData)
            }

        /**
         * 保存设定了闹钟的推送ID，以便在closePush()方法里面清理
         *
         * @param type
         * @param notificationId
         */
        fun appendNotificationIdToPrefenence(type: Int, notificationId: Long) {
            val idJson = AppPreference.getPushNotificationIds()
            val idContent = type.toString() + "_" + notificationId
            if (!idJson?.contains(idContent)!!) {
                val builder = StringBuilder(idJson)
                if (!TextUtils.isEmpty(idJson)) {
                    builder.append(",")
                }
                builder.append(idContent)
                AppPreference.savePushNotificationIds(builder.toString())
            }
        }

        val notificationIdsFromPreference: List<String?>
            get() {
                val idJson = AppPreference.getPushNotificationIds()
                var idList: List<String?> = ArrayList()
                if (!idJson.isNullOrEmpty()) {
                    val ids = idJson.split(",".toRegex()).dropLastWhile { it.isEmpty() }
                        .toTypedArray()
                    idList = listOf(*ids)
                }
                return idList
            }

        fun clearNotificationIds() {
            AppPreference.savePushNotificationIds("")
        }
    }
}
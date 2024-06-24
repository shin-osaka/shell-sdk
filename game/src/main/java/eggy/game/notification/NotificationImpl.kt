package eggy.game.notification

import android.app.AlarmManager
import android.app.AlarmManager.AlarmClockInfo
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import eggy.game.notification.PushData.PopType
import eggy.util.FormatUtil

class NotificationImpl(private val mContext: Context) : AbstractNotification() {
    override fun closePush() {
        val idList =
            NotificationUtil.notificationIdsFromPreference //已经通过sendPushNewsWithValue设定的闹钟ID。
        val pushDataList = NotificationUtil.localPushDataList //服务器配置的推送列表
        val builder = StringBuilder()
        for (i in idList.indices) {
            val idContent = idList[i]
            if (!idContent.isNullOrEmpty()) {
                val idParts =
                    idContent.split("_".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
                if (idParts.size == 2) {
                    val type = FormatUtil.parseInt(idParts[0])
                    val notificationId = FormatUtil.parseInt(idParts[1])
                    if (type == PopType.TYPE_ONE_TIME) {
                        cancelNotificationById(notificationId)
                        builder.append("$idContent,")
                    } else if (type == PopType.TYPE_REPEAT) {
                        if (!isPushExistOnServer(pushDataList, notificationId)) {
                            cancelNotificationById(notificationId)
                            builder.append("$idContent,")
                        }
                    }
                }
            }
        }
        NotificationUtil.clearNotificationIds()
    }

    private fun cancelNotificationById(notificationId: Int) {
        val intent = Intent(mContext, NotificationService::class.java)
        val pendingIntent = PendingIntent.getService(
            mContext,
            notificationId,
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )
        val alarmManager = mContext.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        alarmManager.cancel(pendingIntent)
    }

    private fun isPushExistOnServer(pushDataList: PushDataList?, notificationId: Int): Boolean {
        var found = false
        if (null != pushDataList) {
            for (i in pushDataList.list.indices) {
                val pushData = pushDataList.list[i]
                val popTypes = pushData.popTypes
                for (j in popTypes.indices) {
                    val popType = popTypes[j]
                    val values = popType.longValues
                    val type = popType.type
                    for (k in values.indices) {
                        val value = values[k]
                        val tempId = NotificationUtil.buildLocalPushId(pushData.id, type, value)
                        if (notificationId == tempId) {
                            found = true
                            break
                        }
                    }
                }
            }
        }
        return found
    }

    override fun sendPushNewsWithValue(
        notificationIdString: String?,
        triggerAtMillisStr: String?,
        title: String?,
        content: String?,
        pushInfoStr: String?
    ) {
        if (content.isNullOrEmpty()) {
            return
        }
        var type = 0
        var value: Long = 0
        if (!pushInfoStr.isNullOrEmpty()) {
            val pushInfos =
                pushInfoStr.split("_".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
            if (pushInfos.size >= 3) {
                type = FormatUtil.parseInt(pushInfos[1])
                value = FormatUtil.parseLong(pushInfos[2])
            }
        }
        val tag = System.currentTimeMillis()
        val notificationId = FormatUtil.parseInt(notificationIdString!!)
        var triggerTime = FormatUtil.parseLong(triggerAtMillisStr!!) * 1000
        val intent = Intent(mContext, NotificationService::class.java)
        intent.putExtra("title", title)
        intent.putExtra("content", content)
        intent.putExtra("tag", tag)
        intent.putExtra("notificationId", notificationId)
        intent.putExtra("pushInfoStr", pushInfoStr)
        val alarmManager = mContext.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        var shouldSetAlarm = true
        if (type == 2) {
            val sender = PendingIntent.getService(
                mContext,
                notificationId,
                intent,
                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
            )
            if (sender != null) {
                shouldSetAlarm = false
            }
        }
        if (shouldSetAlarm) {
            val pendingIntent = PendingIntent.getService(
                mContext,
                notificationId,
                intent,
                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
            )
            if (type == PopType.TYPE_ONE_TIME) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) { //MARSHMALLOW OR ABOVE
                    alarmManager.setExactAndAllowWhileIdle(
                        AlarmManager.RTC_WAKEUP,
                        triggerTime,
                        pendingIntent
                    )
                } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) { //LOLLIPOP 21 OR ABOVE
                    val alarmClockInfo = AlarmClockInfo(triggerTime, pendingIntent)
                    alarmManager.setAlarmClock(alarmClockInfo, pendingIntent)
                } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) { //KITKAT 19 OR ABOVE
                    alarmManager.setExact(AlarmManager.RTC_WAKEUP, triggerTime, pendingIntent)
                } else { //FOR BELOW KITKAT ALL DEVICES
                    alarmManager[AlarmManager.RTC_WAKEUP, triggerTime] = pendingIntent
                }
            } else if (type == PopType.TYPE_REPEAT) {
                if (value > 0) {
                    val intervalMillis = value * AlarmManager.INTERVAL_DAY
                    triggerTime += intervalMillis // 首次触发时间
                    alarmManager.setInexactRepeating(
                        AlarmManager.RTC_WAKEUP,
                        triggerTime,
                        intervalMillis,
                        pendingIntent
                    )
                }
            }
        }
        NotificationUtil.appendNotificationIdToPrefenence(type, notificationId.toLong())
    }

    companion object {
        val TAG = NotificationImpl::class.java.simpleName
    }
}
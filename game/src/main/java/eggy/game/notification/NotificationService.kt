package eggy.game.notification

import android.app.IntentService
import android.content.Intent

/**
 * 通知服务
 */
class NotificationService : IntentService("NotificationService") {
    override fun onHandleIntent(intent: Intent?) {
        val title = intent!!.extras!!.getString("title")
        val content = intent.extras!!.getString("content")
        val notificationId = intent.extras!!.getInt("notificationId")
        val pushInfoStr = intent.extras!!.getString("pushInfoStr")
        NotificationUtil.init(this).showNotification(notificationId, title, content, pushInfoStr)
    }

    companion object {
        val TAG = NotificationService::class.java.simpleName
    }
}
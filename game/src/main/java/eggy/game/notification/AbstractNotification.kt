package eggy.game.notification

abstract class AbstractNotification {
    abstract fun closePush()
    abstract fun sendPushNewsWithValue(
        notificationIdString: String?,
        triggerAtMillisStr: String?,
        title: String?,
        content: String?,
        pushInfoStr: String?
    )
}
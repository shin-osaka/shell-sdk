package eggy.game.receiver

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import eggy.game.events.CommandEvent
import org.greenrobot.eventbus.EventBus

class NetworkStateReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val commandEvent = CommandEvent("local://command?cmd=network_state_changed")
        EventBus.getDefault().post(commandEvent)
    }

    companion object {
        val TAG = NetworkStateReceiver::class.java.simpleName
    }
}
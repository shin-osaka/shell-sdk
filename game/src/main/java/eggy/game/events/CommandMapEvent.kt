package eggy.game.events

data class CommandMapEvent(val command: String) {
    var data: MutableMap<String, Any?> = HashMap()

    init {
        data = HashMap()
    }

    fun putData(key: String, value: Any) {
        data[key] = value
    }

    override fun toString(): String {
        return "CommandMapEvent{" +
                "command='" + command + '\'' +
                ", data=" + data +
                '}'
    }
}
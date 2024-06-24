package eggy.game.events

data class CommandEvent(val command: String) {

    override fun toString(): String {
        return "CommandEvent{" +
                "command='" + command + '\'' +
                '}'
    }
}
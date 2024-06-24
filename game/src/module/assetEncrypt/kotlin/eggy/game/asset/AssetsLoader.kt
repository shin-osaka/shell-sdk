package eggy.game.asset

object AssetsLoader {

    private val onAssetsLoadListeners: MutableList<OnAssetsLoadListener> = ArrayList()
    fun register(listener: OnAssetsLoadListener) {
        onAssetsLoadListeners.add(listener)
    }

    fun remove(listener: OnAssetsLoadListener) {
        onAssetsLoadListeners.remove(listener)
    }

    fun onAssetLoaded() {
        for (listener in onAssetsLoadListeners) {
            listener.onLoaded()
        }
    }

}
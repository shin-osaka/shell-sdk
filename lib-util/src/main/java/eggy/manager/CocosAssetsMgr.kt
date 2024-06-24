package eggy.manager

import android.content.Context
import android.content.res.AssetManager

object CocosAssetsMgr {
    val TAG: String = CocosAssetsMgr::class.java.simpleName


    private var mAssetManager: AssetManager? = null
    fun onAssetsLoaded(assetManager: AssetManager) {
        mAssetManager = assetManager;
    }

    fun getAssets(context: Context): AssetManager {
        return mAssetManager ?: context.assets
    }

}
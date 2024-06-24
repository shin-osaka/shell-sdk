package eggy.game.sdk

import android.content.Context
import eggy.game.BuildConfig
import eggy.game.asset.AssetsLoader
import eggy.game.asset.OnAssetsLoadListener
import eggy.game.asset.AssetsUtil

open class BaseAssetsShellSDK : BaseShellSDK() {


    companion object {
        private var mIsAssetsLoaded = !BuildConfig.ASSET_ENCRYPT_ENABLE
        internal var mCallback: ShellInspectCallback? = null
        internal var mReason = 0


        private val mOnAssetsLoadListener = object : OnAssetsLoadListener {
            override fun onLoaded() {
                mIsAssetsLoaded = true
                postCallback()
            }
        }

        /**
         * init shell-sdk with the information as below:
         * configAssets: config file in assets, including channel, brand, etc
         * bgAssets: background image file in assets showed in hot update loading screen
         * logoAssets: logo image in assets showed in the central of hot update loading screen
         */
        @JvmStatic
        internal fun initSDK(context: Context, config: ShellConfig) {
            AssetsLoader.register(mOnAssetsLoadListener)
            AssetsUtil.decryptAssets(context)
            init(context,config)
        }

        internal fun postCallback() {
            if (mReason != 0 && mIsAssetsLoaded) {
                if (mCallback != null) {
                    if (mReason == ShellInspectResult.RESULT_OK) {
                        mCallback?.onShowBSide()
                    } else {
                        mCallback?.onShowASide(mReason)
                    }
                    mCallback = null
                }
                AssetsLoader.remove(mOnAssetsLoadListener)
            }
        }

    }

}
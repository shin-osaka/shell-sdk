package eggy.game.sdk

import android.app.Application
import android.content.Context

open class BaseAssetsShellSDK : BaseShellSDK() {


    companion object {
        internal var mCallback: ShellInspectCallback? = null
        internal var mReason = 0

        /**
         * init shell-sdk with the information as below:
         * configAssets: config file in assets, including channel, brand, etc
         * bgAssets: background image file in assets showed in hot update loading screen
         * logoAssets: logo image in assets showed in the central of hot update loading screen
         */
        @JvmStatic
        internal fun initSDK(context: Application, config: ShellConfig) {
            init(context,config)
        }

        internal fun postCallback() {
            if (mReason != 0) {
                if (mCallback != null) {
                    if (mReason == ShellInspectResult.RESULT_OK) {
                        mCallback?.onShowBSide()
                    } else {
                        mCallback?.onShowASide(mReason)
                    }
                    mCallback = null
                }
            }
        }

    }

}
package eggy.game.manager

import android.app.Activity
import android.content.Context
import android.net.Uri
import android.text.TextUtils
import eggy.util.AppGlobal
import eggy.util.LogUtil
import com.osaka.sdk.Adjust
import com.osaka.sdk.AdjustConfig
import com.osaka.sdk.AdjustEvent
import com.osaka.sdk.LogLevel
import com.osaka.sdk.OnAttributionChangedListener
import com.osaka.sdk.OnDeeplinkResponseListener
import com.osaka.sdk.OnEventTrackingFailedListener
import com.osaka.sdk.OnEventTrackingSucceededListener
import com.osaka.sdk.OnSessionTrackingFailedListener
import com.osaka.sdk.OnSessionTrackingSucceededListener
import eggy.game.util.AppUtils
import java.util.Deque
import java.util.LinkedList

object AdjustManager {
    val TAG = AdjustManager::class.java.simpleName
    private val delayEvent: Deque<AdjustEventBean> = LinkedList()
    private var isInit = false

    fun init(context: Context?, appToken: String?) {
        var environment = AdjustConfig.ENVIRONMENT_PRODUCTION
        if (AppUtils.isLoggable) {
            environment = AdjustConfig.ENVIRONMENT_SANDBOX
        }
        val config = AdjustConfig(context, appToken, environment)
        config.apply {
            setLogLevel(LogLevel.VERBOSE)
            onAttributionChangedListener =
                OnAttributionChangedListener { attribution ->
                    if (attribution != null)
                        refreshAdId(attribution.adid)
                }
            onEventTrackingSucceededListener =
                OnEventTrackingSucceededListener { eventSuccessResponseData ->
                    if (eventSuccessResponseData != null)
                        refreshAdId(eventSuccessResponseData.adid)
                }
            onEventTrackingFailedListener =
                OnEventTrackingFailedListener { eventFailureResponseData ->
                    if (eventFailureResponseData != null)
                        refreshAdId(eventFailureResponseData.adid)
                }
            onSessionTrackingSucceededListener =
                OnSessionTrackingSucceededListener { sessionSuccessResponseData ->
                    if (sessionSuccessResponseData != null)
                        refreshAdId(sessionSuccessResponseData.adid)
                }
            onSessionTrackingFailedListener =
                OnSessionTrackingFailedListener { sessionFailureResponseData ->
                    if (sessionFailureResponseData != null)
                        refreshAdId(sessionFailureResponseData.adid)
                }
            onDeeplinkResponseListener = OnDeeplinkResponseListener { true }
            isSendInBackground = true
        }

        Adjust.onCreate(config)
        initAdjustCallback()
        isInit = true
        uploadDelayEvent()
    }

    /**
     * 上传未初始化AdjustSDK就提前上报的事件
     */
    private fun uploadDelayEvent() {
        if (!delayEvent.isEmpty()) {
            for (bean in delayEvent) {
                LogUtil.d(TAG, "uploadDelayEvent")
                trackEvent(bean.key, bean.value)
            }
            delayEvent.clear()
        }
    }

    private fun initAdjustCallback() {
        AppGlobal.application.registerActivityLifecycleCallbacks(object :
            SimpleActivityLifecycleCallback() {
            override fun onActivityResumed(activity: Activity) {
                onResume()
            }

            override fun onActivityPaused(activity: Activity) {
                onPause()
            }
        })
    }

    private fun refreshAdId(newAdId: String?) {
        val oldAdId = AdjustPreference.adjustAdId
        if (!newAdId.isNullOrEmpty() && newAdId != oldAdId) {
            AdjustPreference.saveAdjustAdId(newAdId)
        }
    }

    fun onResume() {
        if (!isInit) {
            LogUtil.d(TAG, "Adjust not init")
            return
        }
        Adjust.onResume()
    }

    fun onPause() {
        if (!isInit) {
            LogUtil.d(TAG, "Adjust not init")
            return
        }
        Adjust.onPause()
    }

    fun onTerminate() {
        isInit = false
    }

    fun appWillOpenUrl(url: Uri?, context: Context?) {
        if (context == null || url == null) return
        if (!isInit) {
            LogUtil.d(TAG, "Adjust not init")
            return
        }
        Adjust.appWillOpenUrl(url, context)
    }

    fun trackEvent(eventToken: String?, callbackParameters: Map<String?, String?>?) {
        if (TextUtils.isEmpty(eventToken)) return
        if (!isInit) {
            LogUtil.d(TAG, "Adjust not init")
            //adjust没有初始化时，用Deque存储事件数据，等初始化完成统一上报
            delayEvent.offer(AdjustEventBean(eventToken, callbackParameters))
            LogUtil.d(TAG, "Save delay event")
            return
        }
        val adjustEvent = AdjustEvent(eventToken?.trim { it <= ' ' })
        if (!callbackParameters.isNullOrEmpty()) {
            val entries = callbackParameters.entries
            val iterator = entries.iterator()
            while (iterator.hasNext()) {
                val (key, value) = iterator.next()
                adjustEvent.addCallbackParameter(key, value)
            }
        }
        Adjust.trackEvent(adjustEvent)
    }

    fun initGoogleAdId(context: Context?) {
        val startTime = System.currentTimeMillis()
        Adjust.getGoogleAdId(context) { id ->
            val costTime = System.currentTimeMillis() - startTime
            val googleAdId = id ?: ""
            LogUtil.d(TAG, "AdjustManager: onGoogleAdIdRead=$googleAdId costTime=${costTime}ms")
            AdjustPreference.saveGpsAdId(googleAdId)
        }
    }
}
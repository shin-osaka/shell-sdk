package eggy.game.base

import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.os.Bundle

interface SDKInterface {
    fun init(context: Context?)
    fun setGLSurfaceView(view: GLSurfaceView?)
    fun onResume()
    fun onPause()
    fun onDestroy()
    fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?)
    fun onNewIntent(intent: Intent?)
    fun onRestart()
    fun onStop()
    fun onBackPressed()
    fun onConfigurationChanged(newConfig: Configuration?)
    fun onRestoreInstanceState(savedInstanceState: Bundle?)
    fun onSaveInstanceState(outState: Bundle?)
    fun onStart()

    companion object {
        val TAG = SDKInterface::class.java.simpleName
    }
}
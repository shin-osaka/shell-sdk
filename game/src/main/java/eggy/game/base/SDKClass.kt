package eggy.game.base

import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.os.Bundle

abstract class SDKClass : SDKInterface {
    var context: Context? = null
        private set

    override fun init(context: Context?) {
        this.context = context
    }

    override fun setGLSurfaceView(view: GLSurfaceView?) {}
    override fun onResume() {}
    override fun onPause() {}
    override fun onDestroy() {}
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {}
    override fun onNewIntent(intent: Intent?) {}
    override fun onRestart() {}
    override fun onStop() {}
    override fun onBackPressed() {}
    override fun onConfigurationChanged(newConfig: Configuration?) {}
    override fun onRestoreInstanceState(savedInstanceState: Bundle?) {}
    override fun onSaveInstanceState(outState: Bundle?) {}
    override fun onStart() {}

    companion object {
        val TAG = SDKClass::class.java.simpleName
    }
}
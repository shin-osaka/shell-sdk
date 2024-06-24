/****************************************************************************
 * Copyright (c) 2010-2013 cocos2d-x.org
 * Copyright (c) 2013-2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * http://www.cocos2d-x.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package eggy.cocos2dx.lib

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Build

class Cocos2dxAccelerometer(mContext: Context) : SensorEventListener {
    private val mSensorManager: SensorManager = mContext.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val mAcceleration: Sensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
    private val mAccelerationIncludingGravity: Sensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION)
    private val mGyroscope: Sensor = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)
    private var mSamplingPeriodUs = SensorManager.SENSOR_DELAY_GAME

    inner class Acceleration {
        var x = 0.0f
        var y = 0.0f
        var z = 0.0f
    }

    inner class RotationRate {
        var alpha = 0.0f
        var beta = 0.0f
        var gamma = 0.0f
    }

    inner class DeviceMotionEvent {
        var acceleration: Acceleration = Acceleration()
        var accelerationIncludingGravity: Acceleration = Acceleration()
        var rotationRate = RotationRate()
    }

    val deviceMotionEvent = DeviceMotionEvent()

    fun enable() {
        mSensorManager.registerListener(this, mAcceleration, mSamplingPeriodUs)
        mSensorManager.registerListener(this, mAccelerationIncludingGravity, mSamplingPeriodUs)
        mSensorManager.registerListener(this, mGyroscope, mSamplingPeriodUs)
    }

    fun disable() {
        mSensorManager.unregisterListener(this)
    }

    fun setInterval(interval: Float) {
        if (Build.VERSION.SDK_INT >= 11) {
            mSamplingPeriodUs = (interval * 1000000).toInt()
        }
        disable()
        enable()
    }

    override fun onSensorChanged(sensorEvent: SensorEvent) {
        val type = sensorEvent.sensor.type
        when (type) {
            Sensor.TYPE_ACCELEROMETER -> {
                deviceMotionEvent.accelerationIncludingGravity.x = sensorEvent.values[0]
                deviceMotionEvent.accelerationIncludingGravity.y = sensorEvent.values[1]
                deviceMotionEvent.accelerationIncludingGravity.z = sensorEvent.values[2]
            }
            Sensor.TYPE_LINEAR_ACCELERATION -> {
                deviceMotionEvent.acceleration.x = sensorEvent.values[0]
                deviceMotionEvent.acceleration.y = sensorEvent.values[1]
                deviceMotionEvent.acceleration.z = sensorEvent.values[2]
            }
            Sensor.TYPE_GYROSCOPE -> {
                deviceMotionEvent.rotationRate.alpha =
                    Math.toDegrees(sensorEvent.values[0].toDouble()).toFloat()
                deviceMotionEvent.rotationRate.beta =
                    Math.toDegrees(sensorEvent.values[1].toDouble()).toFloat()
                deviceMotionEvent.rotationRate.gamma =
                    Math.toDegrees(sensorEvent.values[2].toDouble()).toFloat()
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {}

    companion object {
        private val TAG = Cocos2dxAccelerometer::class.java.simpleName
        @JvmStatic
        external fun onSensorChanged(x: Float, y: Float, z: Float, timestamp: Long)
    }
}
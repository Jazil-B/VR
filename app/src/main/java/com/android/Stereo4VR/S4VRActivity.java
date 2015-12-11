/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.Stereo4VR;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaPlayer;
import android.os.Bundle;


public class S4VRActivity extends Activity implements SensorEventListener{

    S4VRView mView;
    SensorManager sensorManager;
    Sensor sensor;
    public MediaPlayer backgroundMusic;
    static MediaPlayer wolfMusic;
    static MediaPlayer owlMusic;
    static Time temps = new Time();


    public boolean on = true;

    @Override protected void onCreate(Bundle icicle) {

        //long start = System.currentTimeMillis();
       // int temps = 5;

        super.onCreate(icicle);
        mView = new S4VRView(getApplication(), getAssets());
	    setContentView(mView);

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
       // sensor = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);

        backgroundMusic = MediaPlayer.create(this, R.raw.creepy_ambient);
        wolfMusic = MediaPlayer.create(this, R.raw.wolf);
        owlMusic = MediaPlayer.create(this, R.raw.owl);

        backgroundMusic.start();
        backgroundMusic.setLooping(true);

        temps.start();




    }

    void active_chouette(){
        owlMusic.start();

    }


    void active_loup(){
        wolfMusic.start();

    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();/*
        sensorManager.registerListener(gyroListener, sensor,
                SensorManager.SENSOR_DELAY_NORMAL);*/
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION), SensorManager.SENSOR_DELAY_FASTEST);

        mView.onResume();

    }

/*

    public SensorEventListener gyroListener = new SensorEventListener() {*/
        public void onAccuracyChanged(Sensor sensor, int acc) { }

        public void onSensorChanged(SensorEvent event) {

            if (event.accuracy == SensorManager.SENSOR_STATUS_UNRELIABLE)
            {
                return;
            }

            float x = event.values[0];
            float y = event.values[2];
            float z = event.values[1];

           // System.out.println("X : " + (int) x + " Y : " + (int) y + " Z : " + (int) z );
            S4VRLib.gyro(x,y,z);



        }
  /*  };*/



}

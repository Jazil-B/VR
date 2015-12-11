package com.android.Stereo4VR;

import android.media.MediaPlayer;

/**
 * Created by jazz on 10/11/15.
 */


    public class Time extends Thread{
    int Temps_choutte = 35;
    int Temps_loups = 20;
    int delay = 0;
    public MediaPlayer owlMusic;
    S4VRActivity zik = new S4VRActivity();


    public void run() {
        //long start = System.currentTimeMillis();
        int i = 0;
        // boucle tant que la durée de vie du thread est < à 1 minutes
        //while (System.currentTimeMillis() <= (start + (1000 * Temps))) {
          for (;;){
            // traitement
            System.out.println("temps du thread " + (delay) + " sec");

            delay ++;
               if(delai_jeu()%Temps_choutte==0) {
                   zik.active_chouette();
               }

              if(delai_jeu()%Temps_loups==0) {
                  zik.active_loup();
              }

              try {
                // pause
                Thread.sleep(1000);
            } catch (InterruptedException ex) {

            }
        }
    }

    public int delai_jeu() {

        return delay;
    }
}


/* Implémentation de fonctions/macros pour le traitement d'images
 * ( ici particulièrement une convulution de la forme :
 * 0 1 0
 * 1 1 1
 * 0 1 0 )
 *
 * @author Farès BELHADJ (see COPYING) amsi@ai.univ-paris8.fr
 * @date 26 October, 2015
 * */
#include <string.h>
#include <stdlib.h>
#include <jni.h>

#define ALPHA(x) (((x) >> 24) & 0xFF)
#define RED(x)   (((x) >> 16) & 0xFF)
#define GREEN(x) (((x) >>  8) & 0xFF)
#define BLUE(x)  (((x)      ) & 0xFF)
#define COLOR(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
JNIEXPORT void JNICALL  Java_com_noalien_jniblur_jniInterface_convolute(JNIEnv * env, jobject this, jintArray pixels, jint pw, jint ph) {
    int x, y, ypw, ec = 0;
    unsigned long r, g, b, moi, voisinH, voisinB, voisinG, voisinD;
    jint * ppixels = NULL, * temp = NULL;
    if ((ppixels = (*env)->GetIntArrayElements(env, pixels, NULL)) == NULL) { ec = -1; goto convolute_endFlag; }
    if((temp = malloc(pw * ph * sizeof *temp)) == NULL) { ec = -2; goto convolute_endFlag; }
    for(x = 0, ypw = (ph - 1) * pw; x < pw; x++) {
        temp[x] = ppixels[x];
        temp[ypw + x] = ppixels[ypw + x];
    }
    for(y = 0; y < ph; y++) {
        temp[(ypw = y * pw)] = ppixels[ypw];
        temp[ypw + pw - 1] = ppixels[ypw + pw - 1];
    }
    for(y = 1; y < ph - 1; y++) {
        for(x = 1; x < pw - 1; x++) {
            moi     = ppixels[(ypw = y * pw) + x];
            voisinG = ppixels[ypw + x - 1];
            voisinD = ppixels[ypw + x + 1];
            voisinH = ppixels[ypw - pw + x];
            voisinB = ppixels[ypw + pw + x];
            r = (RED(voisinH) + RED(voisinB) + RED(voisinG) + RED(voisinD) + RED(moi)) / 5;
            g = (GREEN(voisinH) + GREEN(voisinB) + GREEN(voisinG) + GREEN(voisinD) + GREEN(moi)) / 5;
            b = (BLUE(voisinH) + BLUE(voisinB) + BLUE(voisinG) + BLUE(voisinD) + BLUE(moi)) / 5;
            temp[ypw + x] = COLOR(255, r, g, b);
        }
    }
    memcpy(ppixels, temp, pw * ph * sizeof *temp);
convolute_endFlag:
    if(temp) free(temp);
    if(ppixels) (*env)->ReleaseIntArrayElements(env, pixels, ppixels, 0);
}



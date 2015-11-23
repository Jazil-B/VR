#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include "externals/include/SDL2/SDL.h"
#include "externals/include/SDL2/SDL_image.h"

#include "GL4D/gl4droid.h"

#include "AssetTool.h"

static GLuint _program;
static GLuint _vPositionHandle, _vTextureHandle, _vNormalHandle;
static GLuint _myTextureHandle;
static GLuint _pause = 0;
static GLfloat _width = 1.0f, _height = 1.0f;
static GLfloat _ratio_x = 1.0f, _ratio_y = 1.0f;

static GLint _solTexture;

// Par exemple si tu veux charger sol.png faudrat faire loadTexture("sol.png")
static GLint loadTexture(const GLchar *filename) {
    GLint idTexture = 0;

    AssetRessource assetRessource = openAsset(filename);
    SDL_RWops *rw = SDL_RWFromMem(assetRessource.buffer, assetRessource.size);
    closeAsset(assetRessource);
    SDL_Surface *image = IMG_Load_RW(rw, 1);

    glGenTextures(1, &idTexture);
    glBindTexture(GL_TEXTURE_2D, idTexture);

    switch (image->format->format)
    {
        case 386930691:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
            break;
        case 376840196:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
            break;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(image);

    return idTexture;
}

static void reshape_one_view_port(int w, int h) {
    GLfloat rx = (_width = w) / (_height = h), ry = 1.0f / rx;
    rx = MAX(rx, 1.0f);
    ry = MAX(ry, 1.0f);
    __android_log_print(ANDROID_LOG_INFO, "Rechape", "width = %f, height = %f, ratio_x = %f, ratio_y = %f\n", _width, _height, rx, ry);
    glViewport(0, 0, w, h);
    gl4duBindMatrix("projmat");
    gl4duLoadIdentityf();
    gl4duFrustumf(-rx, rx, -ry, ry, 2.0f, 1000.0f);
}

static void reshape(int w, int h) {
    const GLfloat minEyeDist = 12.0f; /* on prend des centimètres */
    const GLfloat maxEyeDist = 10000.0f; /* 100 mètres */
    const GLfloat nearSide = minEyeDist * 0.423f * 2.0f; /* pour une ouverture centrée horizontale de 50°, 2 * (sin(25°) ~ 0.423) */
    const GLfloat nearSide_2 = nearSide / 2.0f;
    if((_width = w) > (_height = h)) {
        if((w >> 1) > h) { /* n'arrive que si la largeur est plus de 2 fois la hauteur */
            _ratio_x = _width / (2.0f * _height);
            _ratio_y = 1.0f;
        } else {
            _ratio_x = 1.0f;
            _ratio_y = (2.0f * _height) / _width;
        }
    } else {
        if((h >> 1) > w) { /* n'arrive que si la hauteur est plus de 2 fois la largeur */
            _ratio_x = 1.0f;
            _ratio_y = _height / (2.0f * _width);
        } else {
            _ratio_x = (2.0f * _width) / _height;
            _ratio_y = 1.0f;
        }
    }
    gl4duBindMatrix("projmat");
    gl4duLoadIdentityf();
    gl4duFrustumf(-nearSide_2 * _ratio_x, nearSide_2 * _ratio_x, -nearSide_2 * _ratio_y, nearSide_2 * _ratio_y, minEyeDist, maxEyeDist);
}

static int init(const char * vs, const char * fs) {
    _program = gl4droidCreateProgram(vs, fs);
    if (!_program)
        return 0;
    _vPositionHandle = glGetAttribLocation(_program, "vPosition");
    _vTextureHandle = glGetAttribLocation(_program, "vTexture");
    _vNormalHandle = glGetAttribLocation(_program, "vNormal");
    _myTextureHandle = glGetAttribLocation(_program, "myTexture");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl4duGenMatrix(GL_FLOAT, "projmat");
    gl4duGenMatrix(GL_FLOAT, "mmat");
    gl4duGenMatrix(GL_FLOAT, "vmat");
    return 1;
}

static void scene(int duplicate) {
    GLfloat mat[16], lum_pos[3] = {0.0f, 0.0f, -20.0f};
    static int r1 = 0, r2 = 0, r3 = 0;
    const GLfloat gTriangleVertices[] = { -6.5f, 6.5f, 6.5f, 6.5f, -6.5f, -6.5f, 6.5f, -6.5f };
    const GLfloat gTriangleTextures[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
    const GLfloat gTriangleNormales[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

    glVertexAttribPointer(_vPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    glEnableVertexAttribArray(_vPositionHandle);
    glVertexAttribPointer(_vTextureHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleTextures);
    glEnableVertexAttribArray(_vTextureHandle);
    glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, gTriangleNormales);
    glEnableVertexAttribArray(_vNormalHandle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _solTexture);

    /* Matrice du Model */
    gl4duBindMatrix("mmat");
    gl4duLoadIdentityf();
    gl4duPushMatrix();
    gl4duTranslatef(0.0f, 0.0f, -30.0f);
    gl4duRotatef(r1, 1, 0, 0);
    gl4duRotatef(r2, 0, 1, 0);
    memcpy(mat, gl4duGetMatrixData(), sizeof mat);
    MMAT4INVERSE(mat);
    MMAT4TRANSPOSE(mat);
    lum_pos[0] = 10.0f * sin(M_PI * r3 / 180.0f);
    glUniformMatrix4fv(glGetUniformLocation(_program, "tinv_mmat"), 1, GL_FALSE, mat);
    glUniform3fv(glGetUniformLocation(_program, "lum_pos"), 1, lum_pos);
    gl4duSendMatrices();
    gl4duPopMatrix();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    if(!_pause && !duplicate) {
        r1 += 1;
        r2 += 2;
    }
    r3++;
}

static void stereo(GLfloat w, GLfloat h, GLfloat dw, GLfloat dh) {
    const GLfloat eyesSapce = 6.0f; /* 6cm entre les deux yeux */
    const GLfloat eyesSapce_2 = eyesSapce / 2.0f;
    /* Matrices de View */
    gl4duBindMatrix("vmat");
    gl4duLoadIdentityf();
    glViewport(0, 0, w, h);
    gl4duPushMatrix();
    if(_width > _height)
        gl4duLookAtf(-eyesSapce_2, 0.0f, 0.0f, 0.0f, 0.0f, -30.0f, 0.0f, 1.0f, 0.0f);
    else
        gl4duLookAtf(0.0f, -eyesSapce_2, 0.0f, 0.0f, 0.0f, -30.0f, 0.0f, 1.0f, 0.0f);
    scene(0);
    gl4duBindMatrix("vmat");
    gl4duPopMatrix();
    glViewport(dw, dh, w, h);
    gl4duPushMatrix();
    if(_width > _height)
        gl4duLookAtf( eyesSapce_2, 0.0f, 0.0f, 0.0f, 0.0f, -30.0f, 0.0f, 1.0f, 0.0f);
    else
        gl4duLookAtf(0.0f,  eyesSapce_2, 0.0f, 0.0f, 0.0f, -30.0f, 0.0f, 1.0f, 0.0f);
    scene(1);
    gl4duBindMatrix("vmat");
    gl4duPopMatrix();
}

static void draw(void) {
    glUseProgram(_program);
    glClear(GL_COLOR_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    if(_width > _height)
        stereo(_width / 2.0f, _height, _width / 2.0f, 0.0f);
    else
        stereo(_width, _height / 2.0f, 0.0f, _height / 2.0f);
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_init(JNIEnv * env, jobject obj, jstring vshader, jstring fshader) {
    char * vs = (*env)->GetStringUTFChars(env, vshader, NULL);
    char * fs = (*env)->GetStringUTFChars(env, fshader, NULL);
    init(vs, fs);
    (*env)->ReleaseStringUTFChars(env, vshader, vs);
    (*env)->ReleaseStringUTFChars(env, fshader, fs);
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_reshape(JNIEnv * env, jobject obj,  jint width, jint height) {
    reshape(width, height);
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_draw(JNIEnv * env, jobject obj) {
    draw();
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_click(JNIEnv * env, jobject obj) {
    _pause = !_pause;
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_initAssets(JNIEnv * env, jobject obj, jobject assetManager) {
    jniEnv = env;
    jniAssetManager = assetManager;

    _solTexture = loadTexture("sol.jpg");
}
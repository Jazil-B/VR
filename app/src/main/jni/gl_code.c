#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include "externals/include/SDL2/SDL.h"
#include "externals/include/SDL2/SDL_image.h"

#include "GL4D/gl4droid.h"
#include "GL4D/gl4dm.h"

#include "AssetTool.h"

#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, __FILE__, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, __FILE__, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO , __FILE__, __VA_ARGS__)

#define NBARBRE 5000
#define ALPHA(x) (((x) >> 24) & 0xFF)
#define RED(x)   (((x) >> 16) & 0xFF)
#define GREEN(x) (((x) >>  8) & 0xFF)
#define BLUE(x)  (((x)      ) & 0xFF)
#define COLOR(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

static GLuint _program;
static GLuint _vPositionHandle, _vTextureHandle, _vNormalHandle;
static GLuint _myTextureHandle;
static GLuint _pause = 0;
static GLfloat _width = 1.0f, _height = 1.0f;
static GLfloat _ratio_x = 1.0f, _ratio_y = 1.0f;
static GLfloat Taille_map = 500.0f, depX=0.0f , depZ=-420.0f,eyeX=0.0f,eyeY=0.0f,eyeZ=0.0f,  angleX=0.0f,angleY=0.0f,angleZ=0.0f, DdepX;

typedef struct
{
    GLfloat *gTriangleVertices;
    GLfloat *gTriangleTextures;
    GLfloat *gTriangleNormales;

    GLsizei verticesNum;
    GLsizei texturesNum;
    GLsizei normalesNum;

    GL4DMVector position;
    GL4DMVector rotation;
    GL4DMVector scale;

    GLenum drawType;
    GLsizei size;
} Model;

static GLint _skyboxTexture;
static Model *_skyboxModel;

static GLint _solTexture;
static Model *_solModel;

static GLint _MonstreTexture;
static Model *_MonstreModel;

static GLint _arbreTexture[NBARBRE];
static Model *_arbreModel[NBARBRE];


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
    const GLfloat maxEyeDist = 70.0f; /* 100 mètres */
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
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl4duGenMatrix(GL_FLOAT, "projmat");
    gl4duGenMatrix(GL_FLOAT, "mmat");
    gl4duGenMatrix(GL_FLOAT, "vmat");
    return 1;
}


static Model *createSkybox(GLfloat dimx, GLfloat dimy, GLfloat dimz, GLfloat textureRepeat)
{
    Model *newModel = malloc(sizeof *newModel);

    GLfloat gTriangleVertices[] =
            {
                    -dimx, -dimy, dimz,
                    dimx, -dimy, dimz,
                    -dimx, dimy, dimz,
                    -dimx, dimy, dimz,
                    dimx, -dimy, dimz,
                    dimx, dimy, dimz,

                    dimx, -dimy, dimz,
                    dimx, -dimy, -dimz,
                    dimx, dimy, dimz,
                    dimx, dimy, dimz,
                    dimx, -dimy, -dimz,
                    dimx, dimy, -dimz,

                    -dimx, -dimy, -dimz,
                    dimx, -dimy, -dimz,
                    -dimx, dimy, -dimz,
                    -dimx, dimy, -dimz,
                    dimx, -dimy, -dimz,
                    dimx, dimy, -dimz,

                    -dimx, -dimy, dimz,
                    -dimx, -dimy, -dimz,
                    -dimx, dimy, dimz,
                    -dimx, dimy, dimz,
                    -dimx, -dimy, -dimz,
                    -dimx, dimy, -dimz,

                    -dimx, -dimy, dimz,
                    dimx, -dimy, dimz,
                    -dimx, -dimy, -dimz,
                    -dimx, -dimy, -dimz,
                    dimx, -dimy, dimz,
                    dimx, -dimy, -dimz,

                    -dimx, dimy, dimz,
                    dimx, dimy, dimz,
                    -dimx, dimy, -dimz,
                    -dimx, dimy, -dimz,
                    dimx, dimy, dimz,
                    dimx, dimy, -dimz,
            };
    GLfloat gTriangleTextures[] =
            {
                    1.0f, 0.25f,
                    0.75f, 0.25f,
                    1.0f, 0.75f,
                    1.0f, 0.75f,
                    0.75f, 0.25f,
                    0.75f, 0.75f,

                    0.75f, 0.25f,
                    0.50f, 0.25f,
                    0.75f, 0.75f,
                    0.75f, 0.75f,
                    0.50f, 0.25f,
                    0.50f, 0.75f,

                    0.25f, 0.25f,
                    0.50f, 0.25f,
                    0.25f, 0.75f,
                    0.25f, 0.75f,
                    0.50f, 0.25f,
                    0.50f, 0.75f,

                    0.0f, 0.25f,
                    0.25f, 0.25f,
                    0.0f, 0.75f,
                    0.0f, 0.75f,
                    0.25f, 0.25f,
                    0.25f, 0.75f,

                    0.25f, 0.0f,
                    0.50f, 0.0f,
                    0.25f, 0.25f,
                    0.25f, 0.25f,
                    0.50f, 0.0f,
                    0.50f, 0.25f,

                    0.25f, 1.0f,
                    0.50f, 1.0f,
                    0.25f, 0.75f,
                    0.25f, 0.75f,
                    0.50f, 1.0f,
                    0.50f, 0.75f,
            };
    GLfloat gTriangleNormales[] =
            {
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,

                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,

                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,

                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,

                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,

                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
            };

    newModel->gTriangleVertices = malloc(sizeof gTriangleVertices);
    newModel->gTriangleTextures = malloc(sizeof gTriangleTextures);
    newModel->gTriangleNormales = malloc(sizeof gTriangleNormales);

    newModel->verticesNum = sizeof gTriangleVertices;
    newModel->texturesNum = sizeof gTriangleTextures;
    newModel->normalesNum = sizeof gTriangleNormales;

    memcpy(newModel->gTriangleVertices, gTriangleVertices, newModel->verticesNum);
    memcpy(newModel->gTriangleTextures, gTriangleTextures, newModel->texturesNum);
    memcpy(newModel->gTriangleNormales, gTriangleNormales, newModel->normalesNum);

    newModel->position.x = 0;
    newModel->position.y = 0;
    newModel->position.z = 0;

    newModel->rotation.x = 0;
    newModel->rotation.y = 0;
    newModel->rotation.z = 0;

    newModel->scale.x = 1;
    newModel->scale.y = 1;
    newModel->scale.z = 1;

    newModel->drawType = GL_TRIANGLES;
    newModel->size = 36;

    return newModel;
}

static Model *createCube(GLfloat dimx, GLfloat dimy, GLfloat dimz, GLfloat textureRepeat)
{
    Model *newModel = malloc(sizeof *newModel);

    GLfloat gTriangleVertices[] =
            {
                    -dimx, -dimy, dimz,
                    dimx, -dimy, dimz,
                    -dimx, dimy, dimz,
                   /* -dimx, dimy, dimz,
                    dimx, -dimy, dimz,*/
                    dimx, dimy, dimz,

                    dimx, -dimy, dimz,
                    dimx, -dimy, -dimz,
                    dimx, dimy, dimz,
                   /* dimx, dimy, dimz,
                    dimx, -dimy, -dimz,*/
                    dimx, dimy, -dimz,

                     dimx, -dimy, -dimz,
                    -dimx, -dimy, -dimz,
                    dimx, dimy, -dimz,
                   /* -dimx, dimy, -dimz,
                    dimx, -dimy, -dimz,*/
                    -dimx, dimy, -dimz,

                    -dimx, -dimy, -dimz,
                    -dimx, -dimy, dimz,
                    -dimx, dimy, -dimz,
                   /* -dimx, dimy, dimz,
                    -dimx, -dimy, -dimz,*/
                    -dimx, dimy, dimz,

                    -dimx, -dimy, -dimz,
                    dimx, -dimy, -dimz,
                    -dimx, -dimy, dimz,
                   /* -dimx, -dimy, -dimz,
                    dimx, -dimy, dimz,*/
                    dimx, -dimy, dimz,

                    -dimx, dimy, dimz,
                    dimx, dimy, dimz,
                    -dimx, dimy, -dimz,
                 /*   -dimx, dimy, -dimz,
                    dimx, dimy, dimz,*/
                    dimx, dimy, -dimz,
            };



    GLfloat gTriangleTextures[] =
            {

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,

                    0.0, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,


            };
    GLfloat gTriangleNormales[] =
            {
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                   /* 0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,*/
                    0.0f, 0.0f, 1.0f,

                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    /*1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,*/
                    1.0f, 0.0f, 0.0f,

                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                   /* 0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,*/
                    0.0f, 0.0f, -1.0f,

                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                   /* -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,*/
                    -1.0f, 0.0f, 0.0f,

                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                   /* 0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,*/
                    0.0f, -1.0f, 0.0f,

                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                   /* 0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,*/
                    0.0f, 1.0f, 0.0f,
            };

    newModel->gTriangleVertices = malloc(sizeof gTriangleVertices);
    newModel->gTriangleTextures = malloc(sizeof gTriangleTextures);
    newModel->gTriangleNormales = malloc(sizeof gTriangleNormales);

    newModel->verticesNum = sizeof gTriangleVertices;
    newModel->texturesNum = sizeof gTriangleTextures;
    newModel->normalesNum = sizeof gTriangleNormales;

    memcpy(newModel->gTriangleVertices, gTriangleVertices, newModel->verticesNum);
    memcpy(newModel->gTriangleTextures, gTriangleTextures, newModel->texturesNum);
    memcpy(newModel->gTriangleNormales, gTriangleNormales, newModel->normalesNum);

    newModel->position.x = 0;
    newModel->position.y = 0;
    newModel->position.z = 0;

    newModel->rotation.x = 0;
    newModel->rotation.y = 0;
    newModel->rotation.z = 0;

    newModel->scale.x = 1;
    newModel->scale.y = 1;
    newModel->scale.z = 1;

    newModel->drawType = GL_TRIANGLE_STRIP;
    newModel->size = 24;

    return newModel;
}


static Model *createArbre(GLfloat dimx, GLfloat dimy, GLfloat dimz,GLfloat textureRepeat)
{
    Model *newModel = malloc(sizeof *newModel);

    GLfloat gTriangleVertices[] = { -dimx, dimy, 0.01f,
                                    dimx, dimy, 0.01f,
                                    -dimx, -dimy, 0.01f,
                                    dimx, -dimy, 0.01f,
                                    -dimx, -dimy, 0.01f,
                                    dimx, dimy, 0.01f,

                                    -dimx, dimy, -0.01f,
                                    dimx, dimy, -0.01f,
                                    -dimx, -dimy, -0.01f,
                                    dimx, -dimy, -0.01f,
                                    -dimx, -dimy, -0.01f,
                                    dimx, dimy, -0.01f,

                                    0.01f,dimy,dimz,
                                    0.01f,dimy,-dimz,
                                    0.01f,-dimy,dimz,
                                    0.01f,-dimy,-dimz,
                                    0.01f,-dimy,dimz,
                                    0.01f,dimy,-dimz,

                                    -0.01f,dimy,dimz,
                                    -0.01f,dimy,-dimz,
                                    -0.01f,-dimy,dimz,
                                    -0.01f,-dimy,-dimz,
                                    -0.01f,-dimy,dimz,
                                    -0.01f,dimy,-dimz,

    };

    GLfloat gTriangleTextures[] = {
                                    textureRepeat, 0.0f,
                                    0.0f, 0.0f,
                                    textureRepeat, textureRepeat,
                                    0.0f, textureRepeat,
                                    textureRepeat, textureRepeat,
                                    0.0f, 0.0f,

                                    textureRepeat, 0.0f,
                                    0.0f, 0.0f,
                                    textureRepeat, textureRepeat,
                                    0.0f, textureRepeat,
                                    textureRepeat, textureRepeat,
                                    0.0f, 0.0f,

                                    textureRepeat, 0.0f,
                                    0.0f, 0.0f,
                                    textureRepeat, textureRepeat,
                                    0.0f, textureRepeat,
                                    textureRepeat, textureRepeat,
                                    0.0f, 0.0f,

                                    textureRepeat, 0.0f,
                                    0.0f, 0.0f,
                                    textureRepeat, textureRepeat,
                                    0.0f, textureRepeat,
                                    textureRepeat, textureRepeat,
                                    0.0f, 0.0f,
    };

    GLfloat gTriangleNormales[] = { 0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f,

                                    0.0f, 0.0f, -1.0f,
                                    0.0f, 0.0f, -1.0f,
                                    0.0f, 0.0f, -1.0f,
                                    0.0f, 0.0f, -1.0f,
                                    0.0f, 0.0f, -1.0f,
                                    0.0f, 0.0f, -1.0f,

                                    1.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f,

                                    -1.0f, 0.0f, 0.0f,
                                    -1.0f, 0.0f, 0.0f,
                                    -1.0f, 0.0f, 0.0f,
                                    -1.0f, 0.0f, 0.0f,
                                    -1.0f, 0.0f, 0.0f,
                                    -1.0f, 0.0f, 0.0f,
    };

    newModel->gTriangleVertices = malloc(sizeof gTriangleVertices);
    newModel->gTriangleTextures = malloc(sizeof gTriangleTextures);
    newModel->gTriangleNormales = malloc(sizeof gTriangleNormales);

    newModel->verticesNum = sizeof gTriangleVertices;
    newModel->texturesNum = sizeof gTriangleTextures;
    newModel->normalesNum = sizeof gTriangleNormales;

    memcpy(newModel->gTriangleVertices, gTriangleVertices, newModel->verticesNum);
    memcpy(newModel->gTriangleTextures, gTriangleTextures, newModel->texturesNum);
    memcpy(newModel->gTriangleNormales, gTriangleNormales, newModel->normalesNum);

    newModel->position.x = 0;
    newModel->position.y = 0;
    newModel->position.z = 0;

    newModel->rotation.x = 0;
    newModel->rotation.y = 0;
    newModel->rotation.z = 0;

    newModel->scale.x = 1;
    newModel->scale.y = 1;
    newModel->scale.z = 1;

    newModel->drawType = GL_TRIANGLES;
    newModel->size = 24;

    return newModel;
}

static Model *createPlan(GLfloat dimx, GLfloat dimy, GLfloat textureRepeat)
{
    Model *newModel = malloc(sizeof *newModel);

    GLfloat gTriangleVertices[] = { -dimx, dimy, 0.0f,
                                    dimx, dimy, 0.0f,
                                    -dimx, -dimy, 0.0f,
                                    dimx, -dimy, 0.0f };

    GLfloat gTriangleTextures[] = { 0.0f, 0.0f, 0.0f, textureRepeat, textureRepeat, 0.0f, textureRepeat, textureRepeat };
    GLfloat gTriangleNormales[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

    newModel->gTriangleVertices = malloc(sizeof gTriangleVertices);
    newModel->gTriangleTextures = malloc(sizeof gTriangleTextures);
    newModel->gTriangleNormales = malloc(sizeof gTriangleNormales);

    newModel->verticesNum = sizeof gTriangleVertices;
    newModel->texturesNum = sizeof gTriangleTextures;
    newModel->normalesNum = sizeof gTriangleNormales;

    memcpy(newModel->gTriangleVertices, gTriangleVertices, newModel->verticesNum);
    memcpy(newModel->gTriangleTextures, gTriangleTextures, newModel->texturesNum);
    memcpy(newModel->gTriangleNormales, gTriangleNormales, newModel->normalesNum);

    newModel->position.x = 0;
    newModel->position.y = 0;
    newModel->position.z = 0;

    newModel->rotation.x = 0;
    newModel->rotation.y = 0;
    newModel->rotation.z = 0;

    newModel->scale.x = 1;
    newModel->scale.y = 1;
    newModel->scale.z = 1;

    newModel->drawType = GL_TRIANGLE_STRIP;
    newModel->size = 4;

    return newModel;
}

static void deleteModel(Model *aModel)
{
    free(aModel->gTriangleVertices);
    free(aModel->gTriangleTextures);
    free(aModel->gTriangleNormales);

    free(aModel);
}

static void displayModel(Model *amodel, GLuint texture)
{
    GLfloat mat[16];
    glEnable(GL_DEPTH_TEST);

    glVertexAttribPointer(_vPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, amodel->gTriangleVertices);
    glEnableVertexAttribArray(_vPositionHandle);
    glVertexAttribPointer(_vTextureHandle, 2, GL_FLOAT, GL_FALSE, 0, amodel->gTriangleTextures);
    glEnableVertexAttribArray(_vTextureHandle);
    glVertexAttribPointer(_vNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, amodel->gTriangleNormales);
    glEnableVertexAttribArray(_vNormalHandle);

    glUniform1i(_myTextureHandle, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    gl4duBindMatrix("mmat");
    gl4duLoadIdentityf();
    gl4duPushMatrix();
    gl4duTranslatef(amodel->position.x, amodel->position.y, amodel->position.z);
    gl4duRotatef(amodel->rotation.x, 1, 0, 0);
    gl4duRotatef(amodel->rotation.y, 0, 1, 0);
    gl4duRotatef(amodel->rotation.z, 0, 0, 1);
    gl4duScalef(amodel->scale.x, amodel->scale.y, amodel->scale.z);

    memcpy(mat, gl4duGetMatrixData(), sizeof mat);
    MMAT4INVERSE(mat);
    MMAT4TRANSPOSE(mat);
    glUniformMatrix4fv(glGetUniformLocation(_program, "tinv_mmat"), 1, GL_FALSE, mat);

    gl4duSendMatrices();
    gl4duPopMatrix();

    glDrawArrays(amodel->drawType, 0, amodel->size);
}

float distance(float posX, float posZ){
    return (sqrt((DdepX-posX)*(DdepX-posX)+(depZ-posZ)*(depZ-posZ)));
}

static void scene(int duplicate) {
    GLfloat lum_pos[3] = {0.0f, 0.0f, -20.0f};
    static float r1 = 0, r2 = 0, r3 = 0;
    static float cpt=0;
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);

    /* Matrice du Model */
    //lum_pos[0] = 10.0f * sin(M_PI * r3 / 180.0f);
    glUniform3fv(glGetUniformLocation(_program, "lum_pos"), 1, lum_pos);

    _skyboxModel->rotation.y = r1;

    _MonstreModel->position.y = sin(cpt);

   /* if(cpt%2==0){
        _MonstreTexture = loadTexture("monster1.png");
    }else{
        _MonstreTexture = loadTexture("monster2.png");
    }*/

    displayModel(_skyboxModel, _skyboxTexture);
    displayModel(_solModel, _solTexture);
    displayModel(_MonstreModel, _MonstreTexture);
    for(int i = 0;i<NBARBRE;i++) {
        _arbreModel[i]->rotation.y = r2;
        if(distance(_arbreModel[i]->position.x,_arbreModel[i]->position.z)<=80.0) {
            displayModel(_arbreModel[i], _arbreTexture[i]);
        }
    }
    if(!_pause && !duplicate) {
        r1 += 0.01;
        r2 += 1;
    }
    r3++;
    cpt+=0.1;
}

static void stereo(GLfloat w, GLfloat h, GLfloat dw, GLfloat dh) {
    const GLfloat eyesSapce = 6.0f; /* 6cm entre les deux yeux */
    const GLfloat eyesSapce_2 = eyesSapce / 2.0f;

    /* Matrices de View */
    DdepX=eyesSapce_2+depX;
    gl4duBindMatrix("vmat");
    gl4duLoadIdentityf();
    glViewport(0, 0, w, h);
    gl4duPushMatrix();
    if(_width > _height)
        gl4duLookAtf(-DdepX, 0.0f, depZ, eyeX, eyeY, -30.0f, 0.0f, 1.0f, 0.0f);
    else
        gl4duLookAtf(0.0f, -eyesSapce_2, 0.0f, 0.0f, eyeY, -30.0f, 0.0f, 1.0f, 0.0f);
    scene(0);
    gl4duBindMatrix("vmat");
    gl4duPopMatrix();
    glViewport(dw, dh, w, h);
    gl4duPushMatrix();
    if(_width > _height)
        gl4duLookAtf( DdepX, 0.0f, depZ, eyeX, eyeY, -30.0f, 0.0f, 1.0f, 0.0f);
    else
        gl4duLookAtf(0.0f,  eyesSapce_2, 0.0f, 0.0f, eyeY, -30.0f, 0.0f, 1.0f, 0.0f);
    scene(1);
    gl4duBindMatrix("vmat");
    gl4duPopMatrix();
}
int alea(int a, int b){
    return rand()%(b-a) +a;
}



static void draw(void) {
    glUseProgram(_program);
    glClear(GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
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

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_event(JNIEnv * env, jobject obj, jint x, jint z) {
    //_pause = !_pause;
    depX+=x;
    depZ+=z;
}

JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_gyro(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat z) {
//_pause = !_pause;
 /*
    angleX+=x;
    eyeX=sin(angleX*M_PI);*/
//eyeX = (int)x;
eyeY = -(int)y;
    //depX+=y;

}

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


JNIEXPORT void JNICALL Java_com_android_Stereo4VR_S4VRLib_initAssets(JNIEnv * env, jobject obj, jobject assetManager) {
jniEnv = env;
jniAssetManager = assetManager;

_skyboxTexture = loadTexture("skybox.png");
_skyboxModel = createSkybox(500.0f, 500.0f, 500.0f, 1.0f);

_solTexture = loadTexture("sol2.jpg");
_solModel = createPlan(Taille_map, Taille_map, 50.0f);

_MonstreTexture = loadTexture("monster1.png");
_MonstreModel = createArbre(5.0f, 3.0f,5.0f, 1.0f);


_MonstreModel->position.z = 400;
_MonstreModel->position.x = -3;
_MonstreModel->position.y = 3;



_skyboxModel->rotation.x = 180;

_solModel->position.y = -7;
_solModel->rotation.x = -90;

/*_solModel = createPlan(10, 10, 1.0f);
_solModel->position.z = -10;
_solModel->rotation.x = -95;*/

for(int i = 0;i<NBARBRE;i++){

switch(i%3){

case 0 :
_arbreTexture[i] = loadTexture("arbre.png");
break;
case 1 :
_arbreTexture[i] = loadTexture("arbre2.png");
break;
case 2 :
_arbreTexture[i] = loadTexture("arbre3.png");
break;
default:_arbreTexture[i] = loadTexture("arbre.png");
break;
}

//_arbreTexture[i] = loadTexture("arbre2.png");
_arbreModel[i] = createArbre(2.0f, 5.0f, 2.0f, 1.0f);

_arbreModel[i]->position.z = alea(-(Taille_map-2),(Taille_map-2));
_arbreModel[i]->position.x = alea(-(Taille_map-2),(Taille_map-2));
//_arbreModel[i]->rotation.z = -90;
}



_arbreModel[49]->position.z = -400;
_arbreModel[49]->position.x = -3;
}
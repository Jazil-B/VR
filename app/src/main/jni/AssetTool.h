//
// Created by mzartek on 22/10/15.
//

#ifndef ANDROIDPROJECT_ASSETTOOL_H
#define ANDROIDPROJECT_ASSETTOOL_H

#include <android/asset_manager_jni.h>

#include <stdlib.h>

extern JNIEnv *jniEnv;
extern jobject jniAssetManager;

typedef struct
{
    AAsset *asset;
    long size;
    char *buffer;
} AssetRessource;

AssetRessource openAsset(const char *asset);
void closeAsset(AssetRessource assetRessource);

#endif //ANDROIDPROJECT_ASSETTOOL_H

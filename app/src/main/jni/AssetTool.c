//
// Created by mzartek on 22/10/15.
//

#include "AssetTool.h"

JNIEnv *jniEnv;
jobject jniAssetManager;

AssetRessource openAsset(const char *assetPath)
{
     AssetRessource assetRessource;
     AAssetManager* mgr = AAssetManager_fromJava(jniEnv, jniAssetManager);
     assetRessource.asset = AAssetManager_open(mgr, assetPath, AASSET_MODE_UNKNOWN);
     assetRessource.size = AAsset_getLength(assetRessource.asset);
     assetRessource.buffer = (char *)malloc(assetRessource.size);
     AAsset_read (assetRessource.asset,
		  assetRessource.buffer,
		  assetRessource.size);

     return assetRessource;
}

void closeAsset(AssetRessource assetRessource)
{
     AAsset_close(assetRessource.asset);
}

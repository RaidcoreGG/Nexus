///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxFuncDefs.h
/// Description  :  Function definitions for textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TXFUNCDEFS_H
#define TXFUNCDEFS_H

#include "TxTexture.h"

typedef void       (*TEXTURES_RECEIVECALLBACK)        (const char* aIdentifier, Texture_t* aTexture);
typedef Texture_t* (*TEXTURES_GET)                    (const char* aIdentifier);
typedef Texture_t* (*TEXTURES_GETORCREATEFROMFILE)    (const char* aIdentifier, const char* aFilename);
typedef Texture_t* (*TEXTURES_GETORCREATEFROMRESOURCE)(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);
typedef Texture_t* (*TEXTURES_GETORCREATEFROMURL)     (const char* aIdentifier, const char* aRemote, const char* aEndpoint);
typedef Texture_t* (*TEXTURES_GETORCREATEFROMURL2)    (const char* aIdentifier, const char* aURL);
typedef Texture_t* (*TEXTURES_GETORCREATEFROMMEMORY)  (const char* aIdentifier, void* aData, size_t aSize);
typedef void       (*TEXTURES_LOADFROMFILE)           (const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMRESOURCE)       (const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMURL)            (const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMURL2)           (const char* aIdentifier, const char* aURL, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMMEMORY)         (const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback);

#endif

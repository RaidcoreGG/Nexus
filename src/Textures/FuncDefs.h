#ifndef TEXTURES_FUNCDEFS_H
#define TEXTURES_FUNCDEFS_H

#include <Windows.h>
#include <string>

#include "Texture.h"

typedef void		(*TEXTURES_RECEIVECALLBACK)(const char* aIdentifier, Texture* aTexture);
typedef Texture*	(*TEXTURES_GET)(const char* aIdentifier);
typedef void		(*TEXTURES_LOADFROMFILE)(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);
typedef void		(*TEXTURES_LOADFROMRESOURCE)(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

#endif
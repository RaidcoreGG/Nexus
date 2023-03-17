#ifndef TEXTURES_FUNCDEFS_H
#define TEXTURES_FUNCDEFS_H

#include <Windows.h>
#include <string>

#include "Texture.h"

typedef void		(*TEXTURES_RECEIVECALLBACK)(std::string aIdentifier, Texture* aTexture);
typedef Texture*	(*TEXTURES_GET)(std::string aIdentifier);
typedef void		(*TEXTURES_LOADFROMFILE)(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback);
typedef void		(*TEXTURES_LOADFROMRESOURCE)(std::string aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

#endif
#ifndef QUEUEDTEXTURE_H
#define QUEUEDTEXTURE_H

#include <string>

#include "FuncDefs.h"

/* A structure holding information about a texture that is queued to load. */
struct QueuedTexture
{
	unsigned Width;
	unsigned Height;
	std::string Identifier;
	unsigned char* Data;
	TEXTURES_RECEIVECALLBACK Callback;
};

#endif
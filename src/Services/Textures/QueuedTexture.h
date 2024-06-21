///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QueuedTexture.h
/// Description  :  Contains the QueudTexture data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QUEUEDTEXTURE_H
#define QUEUEDTEXTURE_H

#include <string>

#include "FuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// QueuedTexture data struct
///----------------------------------------------------------------------------------------------------
struct QueuedTexture
{
	unsigned Width;
	unsigned Height;
	std::string Identifier;
	unsigned char* Data;
	TEXTURES_RECEIVECALLBACK Callback;
};

#endif

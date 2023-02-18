#ifndef QUEUEDTEXTURE_H
#define QUEUEDTEXTURE_H

#include <string>

#include "FuncDefs.h"

struct QueuedTexture
{
    unsigned Width;
    unsigned Height;
    std::string Identifier;
    unsigned char* Data;
    TEXTURES_RECEIVECALLBACK Callback;
};

#endif
#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>

struct Texture
{
    unsigned Width;
    unsigned Height;
    ID3D11ShaderResourceView* Resource;
};

#endif
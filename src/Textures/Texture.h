///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Texture.h
/// Description  :  Contains the Texture data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>

///----------------------------------------------------------------------------------------------------
/// Texture data struct
///----------------------------------------------------------------------------------------------------
struct Texture
{
	unsigned Width;
	unsigned Height;
	ID3D11ShaderResourceView* Resource;
};

#endif

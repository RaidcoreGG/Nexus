///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxTexture.h
/// Description  :  Contains the Texture data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TXTEXTURE_H
#define TXTEXTURE_H

#include <d3d11.h>

///----------------------------------------------------------------------------------------------------
/// Texture_t Struct
///----------------------------------------------------------------------------------------------------
struct Texture_t
{
	unsigned                  Width;
	unsigned                  Height;
	ID3D11ShaderResourceView* Resource;
};

#endif

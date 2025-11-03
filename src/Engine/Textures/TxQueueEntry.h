///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxQueueEntry.h
/// Description  :  Contains the QueueEntry struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "TxEnum.h"
#include "TxFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// QueuedTexture_t Struct
///----------------------------------------------------------------------------------------------------
struct QueuedTexture_t
{
	ETextureStage            Stage;
	long long                Time;

	unsigned                 Width;
	unsigned                 Height;
	unsigned char*           Data;
	std::string              DownloadURL;
	TEXTURES_RECEIVECALLBACK Callback;
};

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxQueueEntry.h
/// Description  :  Contains the QueueEntry struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

#include "TxEnum.h"
#include "TxTexture.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Graphics Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Graphics
{
	typedef void (*TEXTURES_RECEIVECALLBACK) (const char* aIdentifier, Texture_t* aTexture);

	///----------------------------------------------------------------------------------------------------
	/// QueuedTexture_t Struct
	///----------------------------------------------------------------------------------------------------
	struct QueuedTexture_t
	{
		ETextureStage            Stage;
		uint64_t                 Time;

		uint32_t                 Width;
		uint32_t                 Height;
		uint8_t*                 Data;
		std::string              DownloadURL;
		TEXTURES_RECEIVECALLBACK Callback;
	};
}

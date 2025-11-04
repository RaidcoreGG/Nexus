///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiEnum.h
/// Description  :  Enumerations for the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

enum class ERenderType : uint32_t
{
	PreRender,
	Render,
	PostRender,
	OptionsRender,

	COUNT
};

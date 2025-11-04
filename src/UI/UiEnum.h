///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiEnum.h
/// Description  :  Enumerations for the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// ERenderType Enumeration
///----------------------------------------------------------------------------------------------------
enum class ERenderType : uint32_t
{
	PreRender,
	Render,
	PostRender,
	OptionsRender,

	COUNT
};

///----------------------------------------------------------------------------------------------------
/// EUIStyle Enumeration
///----------------------------------------------------------------------------------------------------
enum class EUIStyle : uint32_t
{
	User,
	Nexus,
	ImGui_Classic,
	ImGui_Light,
	ImGui_Dark,
	ArcDPS_Default,
	ArcDPS_Current, /* If available. */
	File,
	Code
};

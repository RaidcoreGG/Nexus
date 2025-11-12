///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  SnowflakeMgr.h
/// Description  :  Contains the logic for winter flair.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <unordered_map>
#include <vector>

#include "imgui/imgui.h"

#include "UI/Textures/TxTexture.h"

///----------------------------------------------------------------------------------------------------
/// Snowflake_t Struct
///----------------------------------------------------------------------------------------------------
struct Snowflake_t
{
	Texture_t* Texture_t;
	float    Size;
	float    X;
	float    Y;

	float    FallDuration;

	/* Sway: left to right motion */
	bool     Sway;
	float    SwayDuration;
	float    SwayOffset;
	float    SwayStart;
	float    SwayEnd;
	
	/* Tilt: slight shake */
	bool     Tilt;
	float    TiltDuration;
	float    Angle;
	float    AngleStart;
	float    AngleEnd;
};

///----------------------------------------------------------------------------------------------------
/// CSnowflakeMgr Class
///----------------------------------------------------------------------------------------------------
class CSnowflakeMgr
{
	public:
	CSnowflakeMgr();

	void Update();
	void Clear();

	private:
	bool IsPartyPooper = false;
	bool IsItChristmas = false;

	std::unordered_map<ImGuiID, std::vector<Snowflake_t>> Snowflakes;
};

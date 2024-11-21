///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  SnowflakeMgr.h
/// Description  :  Contains the logic for winter flair.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef SNOWFLAKEMGR_H
#define SNOWFLAKEMGR_H

#include <unordered_map>
#include <vector>

#include "imgui/imgui.h"

#include "Services/Textures/Texture.h"

///----------------------------------------------------------------------------------------------------
/// Snowflake Struct
///----------------------------------------------------------------------------------------------------
struct Snowflake
{
	Texture* Texture;
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
	bool IsItChristmas = false;

	std::unordered_map<ImGuiID, std::vector<Snowflake>> Snowflakes;
};

#endif

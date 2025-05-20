///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DisplayBinds.h
/// Description  :  Contains the definitions for displayed input binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DISPLAYBINDS_H
#define DISPLAYBINDS_H

#include <string>
#include <unordered_map>

#include "Inputs/GameBinds/GbEnum.h"
#include "Inputs/InputBinds/IbMapping.h"
#include "Inputs/InputBinds/IbBindV2.h"

struct InputBindPacked
{
	std::string KeysText;
	IbMapping   Bind;
};

struct InputBindCategory
{
	std::string                                      Name;
	std::unordered_map<std::string, InputBindPacked> InputBinds;
};

struct GameInputBindPacked
{
	std::string Name;
	std::string KeysText;
	InputBind   Bind;

	std::string KeysText2;
	InputBind   Bind2;
};

struct GameInputBindCategory
{
	std::string                                         Name;
	std::unordered_map<EGameBinds, GameInputBindPacked> GameInputBinds;
};

enum class EBindEditType
{
	None,
	Nexus,
	Game,
	Game2
};

#endif

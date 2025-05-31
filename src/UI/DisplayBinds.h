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

struct InputBindPacked_t
{
	std::string KeysText;
	IbMapping_t Bind;
};

struct InputBindCategory_t
{
	std::string                                        Name;
	std::unordered_map<std::string, InputBindPacked_t> InputBinds;
};

struct GameInputBindPacked_t
{
	std::string Name;
	std::string KeysText;
	InputBind_t Bind;

	std::string KeysText2;
	InputBind_t Bind2;
};

struct GameInputBindCategory_t
{
	std::string                                           Name;
	std::unordered_map<EGameBinds, GameInputBindPacked_t> GameInputBinds;
};

enum class EBindEditType
{
	None,
	Nexus,
	Game,
	Game2
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DisplayBinds.h
/// Description  :  Contains the definitions for displayed input binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DISPLAYBINDS_H
#define DISPLAYBINDS_H

#include "Inputs/GameBinds/EGameBinds.h"
#include "Inputs/InputBinds/ManagedInputBind.h"

struct InputBindPacked
{
	std::string                               KeysText;
	ManagedInputBind                          Bind;
};

struct InputBindCategory
{
	std::string                               Name;
	std::map<std::string, InputBindPacked>    InputBinds;
};

struct GameInputBindPacked
{
	std::string                               Name;
	std::string                               KeysText;
	InputBind                                 Bind;
};

struct GameInputBindCategory
{
	std::string                               Name;
	std::map<EGameBinds, GameInputBindPacked> GameInputBinds;
};

enum class EBindEditType
{
	None,
	Nexus,
	Game
};

#endif

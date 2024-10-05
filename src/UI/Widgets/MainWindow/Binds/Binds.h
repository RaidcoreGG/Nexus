///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Binds.h
/// Description  :  Contains the content of the binds window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_BINDS_H
#define MAINWINDOW_BINDS_H

#include <map>
#include <string>
#include <vector>

#include "Inputs/GameBinds/EGameBinds.h"
#include "Inputs/InputBinds/ManagedInputBind.h"
#include "UI/Controls/CtlSubWindow.h"

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

class CBindsWindow : public ISubWindow
{
	public:
	CBindsWindow();
	void RenderContent() override;
	void RenderSubWindows() override;

	private:
	EBindEditType                              IsEditing = EBindEditType::None;
	std::string                                Editing_Identifier;
	EGameBinds                                 Editing_GameIdentifier;
	std::string                                Editing_BindText;

	std::string                                ModalTitle;
	bool                                       OpenModalNextFrame;

	std::vector<InputBindCategory>             IBCategories;
	std::vector<GameInputBindCategory>         GIBCategories;

	void RenderInputBindsTable(const std::map<std::string, InputBindPacked>& aInputBinds);
	void RenderGameInputBindsTable(const std::map<EGameBinds, GameInputBindPacked>& aInputBinds);

	void PopulateInputBinds();
	void PopulateGameInputBinds();

	void DrawBindSetterModal();

	void DeleteStaleBind(const std::string& aIdentifier);
};

#endif

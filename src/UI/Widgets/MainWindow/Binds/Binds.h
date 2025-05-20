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

#include "Inputs/GameBinds/GbEnum.h"
#include "Inputs/InputBinds/IbMapping.h"
#include "UI/Controls/CtlSubWindow.h"
#include "UI/DisplayBinds.h"
#include "BindSetterModal.h"

class CBindsWindow : public ISubWindow
{
	public:
	CBindsWindow();
	void RenderContent() override;
	void RenderSubWindows() override;

	private:
	CBindSetterModal                   BindSetterModal;

	std::vector<InputBindCategory>     IBCategories;
	std::vector<GameInputBindCategory> GIBCategories;

	void RenderInputBindsTable(const std::unordered_map<std::string, InputBindPacked>& aInputBinds);
	void RenderGameInputBindsTable(const std::unordered_map<EGameBinds, GameInputBindPacked>& aInputBinds);

	void DeleteStaleBind(const std::string& aIdentifier);
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Binds.h
/// Description  :  Contains the content of the binds window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <string>
#include <vector>

#include "Host/GW2/Inputs/GameBinds/GbEnum.h"
#include "Engine/Inputs/InputBinds/IbMapping.h"
#include "UI/Controls/CtlSubWindow.h"
#include "UI/UiBinds.h"
#include "BindSetterModal.h"

namespace Raidcore::UI
{
	class CBindsWindow : public UI::ISubWindow
	{
		public:
		CBindsWindow();
		void RenderContent() override;
		void RenderSubWindows() override;

		private:
		CBindSetterModal                   BindSetterModal;

		std::vector<UI::InputBindCategory_t>     IBCategories;
		std::vector<UI::GameInputBindCategory_t> GIBCategories;

		void RenderInputBindsTable(std::unordered_map<std::string, UI::InputBindPacked_t>& aInputBinds);
		void RenderGameInputBindsTable(std::unordered_map<GW2::EGameBinds, UI::GameInputBindPacked_t>& aInputBinds);

		void DeleteStaleBind(const std::string& aIdentifier);
	};
}

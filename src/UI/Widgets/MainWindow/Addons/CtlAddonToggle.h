///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlAddonToggle.h
/// Description  :  Implementation for Load/Unload control.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "imgui.h"
#include "imgui_extensions.h"

#include "Core/Addons/Addon.h"
#include "Core/Context.h"

///----------------------------------------------------------------------------------------------------
/// AddonToggleCtl Namespace
///----------------------------------------------------------------------------------------------------
namespace AddonToggleCtl
{
	///----------------------------------------------------------------------------------------------------
	/// GetButtonText:
	/// 	Returns the text of the toggle state button. E.g. "Load" or "Unload".
	///----------------------------------------------------------------------------------------------------
	inline std::string GetButtonText(CAddon* aAddon)
	{
		std::string buttonText;
		/* If addon is busy. */
		if (aAddon->IsRunningAction())
		{
			buttonText = "...";
		}
		else /* Addon is not busy. */
		{
			if (aAddon->IsLoaded())
			{
				buttonText = "((Unload))";
			}
			else
			{
				buttonText = "((Load))";
			}

			Config_t* config = aAddon->GetConfig();

			if (config && aAddon->IsStateLocked() && (config->LastLoadState != aAddon->IsLoaded()))
			{
				buttonText.append("*");
			}
		}

		return buttonText;
	}

	///----------------------------------------------------------------------------------------------------
	/// Toggle:
	/// 	Actually toggles the state of the addon.
	/// 	Returns true, if the user should be prompted instead.
	///----------------------------------------------------------------------------------------------------
	inline bool Toggle(CAddon* aAddon)
	{
		assert(aAddon->SupportsLoading());

		bool result = false;

		CContext*   ctx    = CContext::GetContext();
		CConfigMgr* cfgmgr = ctx->GetCfgMgr();

		Config_t* config = aAddon->GetConfig();

		/* If addon is busy. */
		if (aAddon->IsRunningAction())
		{
			/* NOP */
		}
		else /* Addon is not busy. */
		{
			if (aAddon->IsLoaded())
			{
				/* Addon is loaded -> Unload */
				aAddon->Unload();
				config->LastLoadState = !config->LastLoadState;
				cfgmgr->SaveConfigs();
			}
			else
			{
				if (aAddon->IsVersionDisabled())
				{
					/* Addon is not loaded, but was version disabled -> Prompt to load */
					result = true;
				}
				else
				{
					/* Addon is not loaded -> Load */
					aAddon->Load();
					config->LastLoadState = !config->LastLoadState;
					cfgmgr->SaveConfigs();
				}
			}
		}

		return result;
	}

	///----------------------------------------------------------------------------------------------------
	/// Tooltip:
	/// 	Renders a tooltip, if the associated element should have one.
	///----------------------------------------------------------------------------------------------------
	inline void Tooltip(CAddon* aAddon)
	{
		if (aAddon->IsStateLocked())
		{
			ImGui::TooltipGeneric("((IsStateLocked))");
		}
	}
}

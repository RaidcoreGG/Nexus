///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoadUnloadButton.h
/// Description  :  Implementation for Load/Unload button.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADUNLOADBUTTON_H
#define LOADUNLOADBUTTON_H

#include <assert.h>
#include <string>

#include "imgui.h"
#include "imgui_extensions.h"

#include "AddonListing.h"
#include "Core/Context.h"

///----------------------------------------------------------------------------------------------------
/// LoadUnloadButton:
/// 	Renders load or unload button and handles the logic.
/// 	Returns true, if load should be confirmed by the user.
///----------------------------------------------------------------------------------------------------
inline bool LoadUnloadButton(AddonListing_t& aAddonListing, float aBtnWidth)
{
	assert(aAddonListing.Addon);

	CContext*      ctx    = CContext::GetContext();
	CConfigMgr*    cfgmgr = ctx->GetCfgMgr();
	CUiContext*    uictx  = ctx->GetUIContext();
	CLocalization* lang   = uictx->GetLocalization();

	Config_t* config = cfgmgr->RegisterConfig(aAddonListing.GetSig());

	std::string buttonText;
	if (aAddonListing.Addon->IsLoaded())
	{
		buttonText = "((Unload))";
	}
	else
	{
		buttonText = "((Load))";
	}

	if (aAddonListing.Addon->IsStateLocked() && (config->LastLoadState != aAddonListing.Addon->IsLoaded()))
	{
		buttonText.append("*");
	}

	bool doPromptLoad = false;

	/* Load/Unload Button */
	if (ImGui::Button(lang->Translate(buttonText.c_str()), ImVec2(aBtnWidth, 0)))
	{
		Config_t* config = cfgmgr->RegisterConfig(aAddonListing.GetSig());

		if (aAddonListing.Addon->IsLoaded())
		{
			/* Addon is loaded -> Unload */
			aAddonListing.Addon->Unload();
			config->LastLoadState = !config->LastLoadState;
			cfgmgr->SaveConfigs();
		}
		else if (!aAddonListing.Addon->IsLoaded() && aAddonListing.Addon->IsVersionDisabled())
		{
			/* Addon is not loaded, but was version disabled -> Prompt to load */
			/* OR addon is not loaded, but volatile game build disparity -> Prompt to load */
			doPromptLoad = true;
		}
		else
		{
			/* Addon is not loaded -> Load */
			aAddonListing.Addon->Load();
			config->LastLoadState = !config->LastLoadState;
			cfgmgr->SaveConfigs();
		}
	}
	if (aAddonListing.Addon->IsStateLocked())
	{
		ImGui::TooltipGeneric(lang->Translate("((IsStateLocked))"));
	}

	return doPromptLoad;
}

#endif

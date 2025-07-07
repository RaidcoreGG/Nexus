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

inline void LoadUnloadButton(AddonListing_t& aAddonListing, float aBtnWidth)
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

	if (aAddonListing.Addon->IsStateLocked() && (config->ShouldLoad != aAddonListing.Addon->IsLoaded()))
	{
		buttonText.append("*");
	}

	/* Load/Unload Button */
	if (ImGui::Button(lang->Translate(buttonText.c_str()), ImVec2(aBtnWidth, 0)))
	{
		if (aAddonListing.Addon->IsLoaded())
		{
			aAddonListing.Addon->Unload();
		}
		else
		{
			aAddonListing.Addon->Load();
		}
		Config_t* config = cfgmgr->RegisterConfig(aAddonListing.GetSig());
		if (config)
		{
			config->ShouldLoad = !config->ShouldLoad;
			cfgmgr->SaveConfigs();
		}
	}
	if (aAddonListing.Addon->IsStateLocked())
	{
		ImGui::TooltipGeneric(lang->Translate("((IsStateLocked))"));
	}
}

#endif

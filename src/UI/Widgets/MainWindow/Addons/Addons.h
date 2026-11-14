///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.h
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <vector>
#include <windows.h>

#include "imgui/imgui.h"

#include "Core/Preferences/PrefSetting.h"
#include "Host/GW2/Addons/AddNotify.h"
#include "AddonListing.h"
#include "CmAddon.h"
#include "LoadConfirmationModal.h"
#include "UI/Controls/CtlSubWindow.h"
#include "UI/Widgets/MainWindow/Binds/BindSetterModal.h"
#include "UninstallConfirmationModal.h"


enum class EAddonsFilterFlags
{
	None,
	ShowEnabled          = 1 << 0,
	ShowDisabled         = 1 << 1,
	ShowDownloadable     = 1 << 2
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonsFilterFlags)

constexpr EAddonsFilterFlags FILTER_INSTALLED
	= EAddonsFilterFlags::ShowEnabled
	| EAddonsFilterFlags::ShowDisabled;

constexpr EAddonsFilterFlags FILTER_LIBRARY = EAddonsFilterFlags::ShowDownloadable;

class CAddonsWindow : public ISubWindow, public IAddonStateNotifyHandler
{
	public:
	CAddonsWindow();
	~CAddonsWindow();
	void Invalidate() override;

	void Invalidate(signed int aAddonID);

	void OnAddonStateNotify(EAddonStateNotify aState, uint32_t aSignature) override;

	private:
	CBindSetterModal            BindSetterModal;
	CUninstallConfirmationModal UninstallConfirmationModal;
	CLoadConfirmationModal      LoadConfirmationModal;
	CAddonContextMenu           AddonContextMenu;

	std::string                 SearchTerm;
	std::vector<AddonListing_t> Addons;
	uint32_t                    AddonsAmtUnfiltered;

	/* Details */
	std::mutex                  Mutex;
	bool                        HasContent = false;
	AddonListing_t              AddonData = {};

	void SetContent(AddonListing_t& aAddonData);
	void ClearContent();

	void AddonItem(AddonListing_t& aAddonData, float aWidth);

	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// RenderFilterBar:
	/// 	Renders the filter bar of the addons window.
	///		aSize parameter is modified to dynamically adjust to window height.
	///----------------------------------------------------------------------------------------------------
	void RenderFilterBar(ImVec2& aSize);

	///----------------------------------------------------------------------------------------------------
	/// RenderBody:
	/// 	Renders the addons list or details view of the addons window.
	///----------------------------------------------------------------------------------------------------
	void RenderBody(ImVec2 aSize);

	///----------------------------------------------------------------------------------------------------
	/// RenderDetails:
	/// 	Renders the details view of a target addon.
	///----------------------------------------------------------------------------------------------------
	void RenderDetails();

	///----------------------------------------------------------------------------------------------------
	/// RenderActionsBar:
	/// 	Renders the actions bar of the addons window.
	///		aSize parameter is modified to dynamically adjust to window height.
	///----------------------------------------------------------------------------------------------------
	void RenderActionsBar(ImVec2& aSize);

	void RenderSubWindows() override;
	void RenderInputBindsTable(const std::unordered_map<std::string, InputBindPacked_t>& aInputBinds);

	void PopulateAddons();

	/* Declared last: they are destroyed first, before the state their change handlers touch. */
	CSetting<EAddonsFilterFlags> Filter    { "AddonFilters", FILTER_INSTALLED, [this](const EAddonsFilterFlags&)
	{
		this->Invalidate();
		this->ClearContent();
	} };
	CSetting<bool>               IsListMode{ "IsListMode", true };
};

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.h
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_ADDONS_H
#define MAINWINDOW_ADDONS_H

#include <mutex>
#include <vector>
#include <windows.h>

#include "imgui/imgui.h"

#include "AddonListing.h"
#include "UI/Controls/CtlSubWindow.h"
#include "UI/Widgets/MainWindow/Binds/BindSetterModal.h"
#include "UninstallConfirmationModal.h"

constexpr const char* OPT_ISLISTMODE = "IsListMode";
constexpr const char* OPT_ADDONFILTERS = "AddonFilters";

enum class EAddonsFilterFlags
{
	None,
	ShowEnabled          = 1 << 0,
	ShowDisabled         = 1 << 1,
	ShowDownloadable     = 1 << 2
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonsFilterFlags)

class CAddonsWindow : public ISubWindow
{
	public:
	CAddonsWindow();
	void Invalidate() override;

	void Invalidate(signed int aAddonID);

	private:
	CBindSetterModal            BindSetterModal;
	CUninstallConfirmationModal UninstallConfirmationModal;

	std::string                 SearchTerm;
	EAddonsFilterFlags          Filter;
	bool                        IsListMode;
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
};

#endif

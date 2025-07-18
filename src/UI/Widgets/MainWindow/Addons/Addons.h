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

#include "DisplayAddon.h"
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
	ShowDownloadable     = 1 << 2,
	ShowInstalled_Arc    = 1 << 3,
	ShowDownloadable_Arc = 1 << 4,
};

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
	std::vector<AddonItemData_t>  Addons;

	/* Details */
	std::mutex                  Mutex;
	bool                        HasContent = false;
	AddonItemData_t               AddonData = {};

	void SetContent(AddonItemData_t& aAddonData);
	void ClearContent();

	void AddonItem(AddonItemData_t& aAddonData, float aWidth);

	void RenderContent() override;
	void RenderSubWindows() override;
	void RenderDetails();
	void RenderInputBindsTable(const std::unordered_map<std::string, InputBindPacked_t>& aInputBinds);

	void PopulateAddons();
};

#endif

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
	std::string                FilterName;
	EAddonsFilterFlags         Filter;
	std::vector<AddonItemData> Addons;

	/* Details */
	std::mutex                 Mutex;
	bool                       HasContent = false;
	AddonItemData              AddonData = {};

	/* Bind Editor */
	EBindEditType              IsEditing = EBindEditType::None;
	std::string                Editing_Identifier;
	EGameBinds                 Editing_GameIdentifier;
	std::string                Editing_BindText;

	std::string                ModalTitle;
	bool                       OpenModalNextFrame;

	void SetContent(AddonItemData& aAddonData);
	void ClearContent();

	void AddonItem(AddonItemData& aAddonData, float aWidth);

	void RenderContent() override;
	void RenderSubWindows() override;
	void RenderDetails();
	void RenderInputBindsTable(const std::map<std::string, InputBindPacked>& aInputBinds);

	void DrawBindSetterModal();

	void PopulateAddons();
};

#endif

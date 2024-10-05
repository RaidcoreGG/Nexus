///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.h
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_ADDONS_H
#define MAINWINDOW_ADDONS_H

#include "UI/Controls/CtlSubWindow.h"
#include "AddonDetails.h"
#include "UI/FuncDefs.h"
#include "Loader/LibraryAddon.h"
#include "Loader/Addon.h"

enum class EAddonType
{
	Nexus,
	Library,
	ArcDPS
};

struct AddonItemData
{
	EAddonType Type;
	union
	{
		Addon* NexusAddon;
		LibraryAddon* LibraryAddon;
		//ArcDPS
	};
	GUI_RENDER OptionsRender;
};

class CAddonsWindow : public ISubWindow
{
	public:
	CAddonsWindow();
	void RenderSubWindows() override;
	void Invalidate() override;

	private:
	CAddonDetailsWindow* Details;

	void RenderContent() override;
};

#endif

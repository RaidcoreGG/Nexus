///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonDetails.h
/// Description  :  Contains the content of the addon details window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_ADDONDETAILS_H
#define MAINWINDOW_ADDONDETAILS_H

#include "UI/Controls/CtlSubWindow.h"

class CAddonDetailsWindow : public ISubWindow
{
	public:
	CAddonDetailsWindow();

	private:
	void RenderContent() override;
};

#endif

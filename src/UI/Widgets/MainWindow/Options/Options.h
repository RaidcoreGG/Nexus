///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Options.h
/// Description  :  Contains the content of the options window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_OPTIONS_H
#define MAINWINDOW_OPTIONS_H

#include <vector>
#include <string>

#include "UI/Controls/CtlSubWindow.h"

class COptionsWindow : public ISubWindow
{
	public:
	COptionsWindow();
	void RenderContent() override;

	private:
	std::vector<std::string> Fonts;

	void TabGeneral();
	void TabStyle();

	void PopulateFonts();
};

#endif

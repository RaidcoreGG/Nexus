///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Options.h
/// Description  :  Contains the content of the options window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <filesystem>

#include "UI/Controls/CtlSubWindow.h"
#include "ExportStyleModal.h"
#include "ImportStyleModal.h"

class COptionsWindow : public ISubWindow
{
	public:
	COptionsWindow();
	void RenderContent() override;
	void RenderSubWindows() override;

	private:
	CExportStyleModal ExportModal;
	CImportStyleModal ImportModal;

	std::vector<std::filesystem::path> Fonts;
	std::vector<std::filesystem::path> Styles;

	bool HasUnsavedStyle = false;

	void TabGeneral();
	void TabStyle();

	void PopulateFonts();
	void PopulateStyles();
};

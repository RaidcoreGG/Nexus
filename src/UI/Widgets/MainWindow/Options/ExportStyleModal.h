///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExportStyleModal.h
/// Description  :  Modal for style exports.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EXPORTSTYLEMODAL_H
#define EXPORTSTYLEMODAL_H

#include <windows.h>
#include <string>
#include <filesystem>

#include "UI/Controls/CtlModal.h"

///----------------------------------------------------------------------------------------------------
/// CExportStyleModal Class
///----------------------------------------------------------------------------------------------------
class CExportStyleModal : public virtual IModal
{
	public:
	CExportStyleModal();

	void RenderContent() override;

	void OnClosing() override;

	void SetData(const std::string& aBase64Style);

	private:
	std::string           Data;
	char                  PathBuffer[MAX_PATH];
	std::filesystem::path Path;

	void ClearData();
};

#endif

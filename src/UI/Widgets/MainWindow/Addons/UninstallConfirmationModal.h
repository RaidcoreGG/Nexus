///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UninstallConfirmationModal.h
/// Description  :  Modal for addon uninstall confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef UNINSTALLCONFIRMATIONMODAL_H
#define UNINSTALLCONFIRMATIONMODAL_H

#include <string>
#include <filesystem>

#include "UI/Controls/CtlModal.h"

///----------------------------------------------------------------------------------------------------
/// CUninstallConfirmationModal Class
///----------------------------------------------------------------------------------------------------
class CUninstallConfirmationModal : public virtual IModal
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUninstallConfirmationModal();

	///----------------------------------------------------------------------------------------------------
	/// RenderContent:
	/// 	Render function of modal contents.
	///----------------------------------------------------------------------------------------------------
	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// OnClosing:
	/// 	Override close to process result and reset variables.
	///----------------------------------------------------------------------------------------------------
	void OnClosing() override;

	///----------------------------------------------------------------------------------------------------
	/// SetTarget:
	/// 	Set which addon to uninstall.
	///----------------------------------------------------------------------------------------------------
	void SetTarget(std::string aName, std::filesystem::path aPath);

	private:
	std::string           Name;
	std::filesystem::path Path;

	///----------------------------------------------------------------------------------------------------
	/// SetTitle:
	/// 	Custom title setting, so that the caption says "Uninstalling: <ADDONNAME>".
	///----------------------------------------------------------------------------------------------------
	void SetTitle();
};

#endif

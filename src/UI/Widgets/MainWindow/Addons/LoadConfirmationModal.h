///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoadConfirmationModal.h
/// Description  :  Modal for addon load confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADCONFIRMATIONMODAL_H
#define LOADCONFIRMATIONMODAL_H

#include <string>
#include <filesystem>

#include "UI/Controls/CtlModal.h"
#include "Core/Addons/Config/Config.h"

///----------------------------------------------------------------------------------------------------
/// CLoadConfirmationModal Class
///----------------------------------------------------------------------------------------------------
class CLoadConfirmationModal : public virtual IModal
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLoadConfirmationModal();

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
	/// 	Set which addon to load.
	///----------------------------------------------------------------------------------------------------
	void SetTarget(Config_t* aConfig, std::string aName, std::filesystem::path aPath);

	private:
	Config_t*             Config = nullptr;
	std::string           Name;
	std::filesystem::path Path;

	///----------------------------------------------------------------------------------------------------
	/// SetTitle:
	/// 	Custom title setting, so that the caption says "Load addon: <ADDONNAME>".
	///----------------------------------------------------------------------------------------------------
	void SetTitle();
};

#endif

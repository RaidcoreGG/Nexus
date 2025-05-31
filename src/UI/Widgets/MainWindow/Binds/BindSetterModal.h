///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BindSetterModal.h
/// Description  :  Modal for InputBind setter.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef BINDSETTERMODAL_H
#define BINDSETTERMODAL_H

#include <string>

#include "UI/Controls/CtlModal.h"
#include "UI/DisplayBinds.h"
#include "Inputs/InputBinds/IbBindV2.h"
#include "Inputs/GameBinds/GbEnum.h"

///----------------------------------------------------------------------------------------------------
/// CBindSetterModal Class
///----------------------------------------------------------------------------------------------------
class CBindSetterModal : public virtual IModal
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CBindSetterModal();

	///----------------------------------------------------------------------------------------------------
	/// RenderContent:
	/// 	Render function of modal contents.
	///----------------------------------------------------------------------------------------------------
	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// OnOpening:
	/// 	Override open to retrieve the bind text of the to be edited bind.
	///----------------------------------------------------------------------------------------------------
	void OnOpening() override;

	///----------------------------------------------------------------------------------------------------
	/// OnClosing:
	/// 	Override close to process result and reset variables.
	///----------------------------------------------------------------------------------------------------
	void OnClosing() override;

	///----------------------------------------------------------------------------------------------------
	/// SetTarget:
	/// 	Set Nexus bind as editing target.
	///----------------------------------------------------------------------------------------------------
	void SetTarget(std::string aBindIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// SetTarget:
	/// 	Set game bind as editing target.
	///----------------------------------------------------------------------------------------------------
	void SetTarget(EGameBinds aBindIdentifier, bool aIsPrimary = true);

	private:
	EBindEditType Type             = EBindEditType::None;
	std::string   NexusBindID      = {};
	EGameBinds    GameBindID       = (EGameBinds)0;
	std::string   PreviousBindText = {};

	InputBind_t   Capture          = {};
	std::string   BindConflict     = {};

	///----------------------------------------------------------------------------------------------------
	/// SetTitle:
	/// 	Custom title setting, so that the caption says "Set Input Bind: <KEYNAME>".
	///----------------------------------------------------------------------------------------------------
	void SetTitle();
};

#endif

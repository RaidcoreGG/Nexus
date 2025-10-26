///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmAddon.h
/// Description  :  Context menu for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef CMADDON_H
#define CMADDON_H

#include "AddonListing.h"
#include "UI/Controls/CtlContextMenu.h"

///----------------------------------------------------------------------------------------------------
/// CLoadConfirmationModal Class
///----------------------------------------------------------------------------------------------------
class CAddonContextMenu : public virtual IContextMenu
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// Render:
	/// 	Callback for the context menu contents.
	///----------------------------------------------------------------------------------------------------
	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// SetContent:
	/// 	Sets the underlying context menu data.
	///----------------------------------------------------------------------------------------------------
	void SetContent(AddonListing_t& aAddonData);

	private:
	AddonListing_t* Data{};
};

#endif

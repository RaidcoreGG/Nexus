///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmAddon.h
/// Description  :  Context menu for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "AddonListing.h"
#include "UI/Controls/CtlContextMenu.h"

namespace Raidcore::UI
{
	///----------------------------------------------------------------------------------------------------
	/// CLoadConfirmationModal Class
	///----------------------------------------------------------------------------------------------------
	class CAddonContextMenu : public virtual UI::IContextMenu
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
}

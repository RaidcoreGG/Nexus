///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MumbleInspector.h
/// Description  :  Contains the content of the mumble data overlay.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "thirdparty/mumble/Mumble.h"

#include "Host/GW2/Addons/API/NexusLink.h"
#include "UI/Controls/CtlWindow.h"

namespace Raidcore::UI
{
	class CMumbleOverlay : public virtual UI::IWindow
	{
		public:
		CMumbleOverlay();

		void Render() override;

		private:
		Mumble::Data*     MumbleLink;
		Mumble::Identity* MumbleIdentity;
		GW2::NexusLinkData_t*    NexusLink;
	};
}

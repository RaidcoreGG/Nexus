///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MumbleInspector.h
/// Description  :  Contains the content of the mumble data overlay.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef OVERLAY_MUMBLEINSPECTOR_H
#define OVERLAY_MUMBLEINSPECTOR_H

#include "Loader/NexusLinkData.h"
#include "Services/Mumble/Definitions/Mumble.h"
#include "UI/Controls/CtlWindow.h"

class CMumbleOverlay : public virtual IWindow
{
	public:
	CMumbleOverlay();

	void Render() override;

	private:
	Mumble::Data*     MumbleLink;
	Mumble::Identity* MumbleIdentity;
	NexusLinkData*    NexusLink;
};

#endif

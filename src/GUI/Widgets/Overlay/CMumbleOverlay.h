#ifndef GUI_MUMBLEOVERLAY_H
#define GUI_MUMBLEOVERLAY_H

#include "UI/Controls/CtlWindow.h"

#include "Loader/NexusLinkData.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Mumble/Definitions/Mumble.h"

class CMumbleOverlay : public IWindow
{
	public:
	CMumbleOverlay(CDataLink* aDataLink);

	void Render();

	private:
	Mumble::Data* MumbleLink;
	Mumble::Identity* MumbleIdentity;
	NexusLinkData* NexusLink;
};

#endif
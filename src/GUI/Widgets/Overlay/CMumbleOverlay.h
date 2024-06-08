#ifndef GUI_MUMBLEOVERLAY_H
#define GUI_MUMBLEOVERLAY_H

#include "GUI/IWindow.h"

#include "Services/Mumble/Definitions/Mumble.h"
#include "Loader/NexusLinkData.h"

namespace GUI
{
	class CMumbleOverlay : public IWindow
	{
	public:
		CMumbleOverlay();

		void Render();

	private:
		Mumble::Data*	MumbleLink;
		NexusLinkData*	NexusLink;
	};
}

#endif
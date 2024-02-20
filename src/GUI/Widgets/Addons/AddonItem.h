#ifndef GUI_ADDONITEM_H
#define GUI_ADDONITEM_H

#include "Loader/Addon.h"
#include "Loader/LibraryAddon.h"

namespace GUI
{
	void AddonItem(Addon* aAddon);
	void AddonItem(LibraryAddon* aAddon, bool aInstalled = false);
}

#endif
#ifndef GUI_ADDONITEM_H
#define GUI_ADDONITEM_H

#include <filesystem>

#include "Loader/Addon.h"
#include "Loader/LibraryAddon.h"

namespace GUI
{
	void AddonItem(std::filesystem::path aPath, Addon* aAddon);
	void AddonItem(LibraryAddon* aAddon, bool aInstalled = false, bool aIsArcPlugin = false);
}

#endif
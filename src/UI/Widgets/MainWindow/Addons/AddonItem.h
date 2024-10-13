#ifdef MEME
#ifndef UI_ADDONITEM_H
#define UI_ADDONITEM_H

#include <filesystem>

#include "Loader/Addon.h"
#include "Loader/LibraryAddon.h"

namespace UIRoot
{
	void AddonItem(std::filesystem::path aPath, Addon* aAddon);
	void AddonItem(LibraryAddon* aAddon, bool aInstalled = false, bool aIsArcPlugin = false);
}

#endif
#endif
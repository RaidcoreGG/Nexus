#ifndef GUI_ADDONITEM_H
#define GUI_ADDONITEM_H

#include "../../../Consts.h"

#include "../../../Loader/Addon.h"
#include "../../../Loader/AddonDefinition.h"
#include "../../../Loader/EAddonFlags.h"
#include "../../../Loader/Loader.h"

#include "../../../Events/EventHandler.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

namespace GUI
{
	void AddonItem(Addon* aAddon);
}

#endif
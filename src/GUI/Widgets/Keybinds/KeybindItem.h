#ifndef GUI_KEYBINDITEM_H
#define GUI_KEYBINDITEM_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../../Keybinds/KeybindHandler.h"
#include "../../../Keybinds/Keybind.h"

namespace GUI
{
	void KeybindItem(std::wstring aIdentifier, Keybind aKeybind);
}

#endif
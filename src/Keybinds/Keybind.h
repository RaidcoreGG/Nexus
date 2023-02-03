#ifndef KEYBIND_H
#define KEYBIND_H

#include <Windows.h>
#include <string>
#include <algorithm>

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

struct Keybind
{
	WORD Key;
	bool Alt;
	bool Ctrl;
	bool Shift;

	std::string ToString(bool padded = false);
};

bool operator==(const Keybind& lhs, const Keybind& rhs);
bool operator!=(const Keybind& lhs, const Keybind& rhs);

Keybind KBFromString(std::string aKeybind);

#endif
#ifndef KEYBIND_H
#define KEYBIND_H

#include <Windows.h>
#include <string>
#include <algorithm>
#include <map>

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

/* A structure holding information about a keybind. */
struct Keybind
{
	WPARAM	Key;
	bool	Alt;
	bool	Ctrl;
	bool	Shift;

	/* Prints the keybind to a string for display. Pass true if spacing/padding should be added. */
	std::string ToString(bool padded = false);
};

bool operator==(const Keybind& lhs, const Keybind& rhs);
bool operator!=(const Keybind& lhs, const Keybind& rhs);

/* Create a keybind from a string. */
Keybind KBFromString(std::string aKeybind);

#endif
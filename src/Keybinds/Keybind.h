#ifndef KEYBIND_H
#define KEYBIND_H

#include <Windows.h>
#include <string>
#include <algorithm>

struct Keybind
{
	WORD Key;
	bool Alt;
	bool Ctrl;
	bool Shift;

	std::wstring ToString();
};

bool operator==(const Keybind& lhs, const Keybind& rhs);
bool operator!=(const Keybind& lhs, const Keybind& rhs);

Keybind KBFromString(std::wstring aKeybind);

#endif
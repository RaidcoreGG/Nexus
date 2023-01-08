#ifndef KEYBIND_H
#define KEYBIND_H

#include <Windows.h>

struct Keybind
{
	WORD Key;
	bool Alt;
	bool Ctrl;
	bool Shift;
};

bool operator==(const Keybind& lhs, const Keybind& rhs);

#endif
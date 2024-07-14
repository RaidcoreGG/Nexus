#ifndef KEYBIND_H
#define KEYBIND_H

#include <string>

/* A structure holding information about a keybind. */
struct Keybind
{
	unsigned short	Key;
	bool			Alt;
	bool			Ctrl;
	bool			Shift;

	bool IsBound();
};

bool operator==(const Keybind& lhs, const Keybind& rhs);
bool operator!=(const Keybind& lhs, const Keybind& rhs);
#endif
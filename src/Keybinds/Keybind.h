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

	/* Prints the keybind to a string for display. Pass true if spacing/padding should be added. */
	std::string ToString(bool padded = false);
};

bool operator==(const Keybind& lhs, const Keybind& rhs);
bool operator!=(const Keybind& lhs, const Keybind& rhs);
#endif
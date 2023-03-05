#ifndef ACTIVEKEYBIND_H
#define ACTIVEKEYBIND_H

#include "Keybind.h"
#include "FuncDefs.h"

struct ActiveKeybind
{
	Keybind Bind;
	KEYBINDS_PROCESS Handler;
};

#endif
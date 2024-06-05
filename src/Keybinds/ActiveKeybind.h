#ifndef ACTIVEKEYBIND_H
#define ACTIVEKEYBIND_H

#include "Keybind.h"
#include "FuncDefs.h"

typedef enum class EKeybindHandlerType
{
	None,
	DownOnly,
	DownAndRelease
} EKBHType;

struct ActiveKeybind
{
	Keybind					Bind;
	EKeybindHandlerType		HandlerType;
	void*					Handler;
};

#endif

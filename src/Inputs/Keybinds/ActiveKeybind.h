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
	std::string				DisplayText;
	Keybind					Bind;
	EKeybindHandlerType		HandlerType;
	void*					Handler;
};

#endif

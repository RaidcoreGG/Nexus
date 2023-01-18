#ifndef MUMBLE_H
#define MUMBLE_H

#include <map>
#include <string>
#include <Windows.h>

#include "LinkedMem.h"

namespace Mumble
{
	LinkedMem* Initialize(const wchar_t* aMumbleName);
	void Shutdown();

	void Update();
};

#endif
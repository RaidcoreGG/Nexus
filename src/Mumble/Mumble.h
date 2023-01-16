#ifndef MUMBLE_H
#define MUMBLE_H

#include <map>
#include <string>
#include <Windows.h>

#include "LinkedMem.h"

namespace Mumble
{
	LinkedMem* Create(const wchar_t* aMumbleName);
	void Destroy();

	void UpdateIdentity();
};

#endif
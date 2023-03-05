#ifndef MUMBLE_H
#define MUMBLE_H

#include <map>
#include <string>
#include <Windows.h>
#include <thread>

#include "../Shared.h"
#include "../State.h"
#include "../Renderer.h"

#include "LinkedMem.h"
#include "../nlohmann/json.hpp"
#include "../Events/EventHandler.h"

namespace Mumble
{
	LinkedMem* Initialize(const wchar_t* aMumbleName);
	void Shutdown();

	HANDLE GetHandle();

	void UpdateIdentity();
	void UpdateState();
};

#endif
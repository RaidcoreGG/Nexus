#ifndef MUMBLE_H
#define MUMBLE_H

#include <map>
#include <string>
#include <Windows.h>
#include <thread>

#include "../Shared.h"
#include "../State.h"
#include "../Renderer.h"
#include "../Consts.h"

#include "LinkedMem.h"
#include "../nlohmann/json.hpp"
#include "../Events/EventHandler.h"

namespace Mumble
{
	/* Initializes the mumble link with the provided name and returns a pointer to it. */
	LinkedMem* Initialize(const wchar_t* aMumbleName);
	/* Frees the mumble link. */
	void Shutdown();

	/* Returns a handle to the mumble link. */
	HANDLE GetHandle();

	/* Loop that polls Mumble->Identity. */
	void UpdateIdentityLoop();
	/* Loop that polls various derived states. */
	void UpdateStateLoop();

	/* Returns the scaling factor for the given the UISize enum. */
	float GetScalingFactor(unsigned aSize);
};

#endif
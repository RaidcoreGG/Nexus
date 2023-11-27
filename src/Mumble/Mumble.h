#ifndef MUMBLE_H
#define MUMBLE_H

#include <thread>

namespace Mumble
{
	extern bool IsRunning;

	extern std::thread UpdateIdentityThread;
	extern std::thread UpdateStateThread;

	/* Initializes the threads parsing the MumbleLink. */
	void Initialize();
	/* Stops the MumbleLink threads. */
	void Shutdown();

	/* Loop that polls Mumble->Identity. */
	void UpdateIdentityLoop();
	/* Loop that polls various derived states. */
	void UpdateStateLoop();

	/* Returns the scaling factor for the given the UISize enum. */
	float GetScalingFactor(unsigned aSize);
};

#endif
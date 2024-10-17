#ifndef STATE_H
#define STATE_H

#include <string>

/* State of Nexus */
enum class ENexusState
{
	NONE,
	LOAD,									/* Nexus is loading */
	LOADED,									/* Nexus has initialized all components */
	READY,									/* Nexus is ready to initialise UI */
	SHUTTING_DOWN,							/* Nexus is shutting down */
	SHUTDOWN								/* Nexus has been shut down */
};

/* State of DirectX */
enum class EDxState
{
	NONE,
	LOAD,							/* Is loading directx (system/chainload) */
	LOADED,							/* Has loaded directx */
	HOOKED,							/* Has installed directx hooks */
	READY							/* Is fully ready (has acquired swapchain) */
};

/* Namespace for global state variables */
namespace State
{
	/* internal states */
	extern ENexusState		Nexus;					/* Nexus state variable */
	extern EDxState			Directx;				/* DirectX state variable */
	extern bool				IsChainloading;			/* Is the Nexus chainloading another proxy dll */
}

#endif
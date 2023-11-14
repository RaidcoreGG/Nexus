#ifndef STATE_H
#define STATE_H

#include <Windows.h>
#include <string>
#include <algorithm>
#include <shellapi.h>

#include "Consts.h"
#include "Shared.h"

#include "Mumble/Mumble.h"
#include "DataLink/DataLink.h"

/* State of Nexus */
enum class ENexusState
{
	NONE,
	LOAD,									/* Nexus is loading */
	LOADED,									/* Nexus has initialized all components */
	READY,									/* Nexus is ready to initialise UI */
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

/* DirectX entry methods */
enum class EEntryMethod
{
	NONE,
	CREATEDEVICE,
	CREATEDEVICEANDSWAPCHAIN,
	CORE_CREATEDEVICE,
	CORE_CREATELAYEREDDEVICE,
	CORE_GETLAYEREDDEVICESIZE,
	CORE_REGISTERLAYERS
};

/* Multibox state */
enum class EMultiboxState
{
	NONE			= 0,
	ARCHIVE_SHARED	= 1,
	LOCAL_SHARED	= 2,
	MUTEX_CLOSED	= 4,
	READY			= ARCHIVE_SHARED | LOCAL_SHARED | MUTEX_CLOSED
};

EMultiboxState operator|(EMultiboxState lhs, EMultiboxState rhs);
EMultiboxState operator&(EMultiboxState lhs, EMultiboxState rhs);

/* Namespace for global state variables */
namespace State
{
	/* internal states */
	extern ENexusState		Nexus;					/* Nexus state variable */
	extern EDxState			Directx;				/* DirectX state variable */
	extern EEntryMethod		EntryMethod;			/* How was the host initialized */
	extern EMultiboxState	MultiboxState;			/* Is this game instance occupying any resources */
	extern bool				IsChainloading;			/* Is the Nexus chainloading another proxy dll*/
	extern bool				IsImGuiInitialized;		/* Is ImGui currently up and running */

	/* start parameters */
	extern bool				IsDeveloperMode;		/* Is Nexus running in developer mode */
	extern bool				IsConsoleEnabled;		/* Is the console window enabled */
	extern bool				IsVanilla;				/* Is Nexus running in vanilla mode and should not load any mods */
	extern bool				IsMumbleDisabled;		/* Is Mumble intentionally disabled */

	void Initialize();
}

#endif
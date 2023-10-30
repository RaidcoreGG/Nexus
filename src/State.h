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
	LOAD,									/* AddonHost is loading */
	LOADED,									/* AddonHost has initialized all components */
	READY,									/* AddonHost is ready to initialise UI */
	SHUTDOWN								/* AddonHost has been shut down */
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
	NONE			= 0x00,
	ARCHIVE_SHARED	= 0x01,
	LOCAL_SHARED	= 0x02,
	MUTEX_CLOSED	= 0x04,
	READY			= ARCHIVE_SHARED | LOCAL_SHARED | MUTEX_CLOSED
};

/* Namespace for global state variables */
namespace State
{
	/* internal states */
	extern ENexusState		Nexus;					/* AddonHost state variable */
	extern EDxState			Directx;				/* DirectX state variable */
	extern EEntryMethod		EntryMethod;			/* How was the host initialized */
	extern EMultiboxState	MultiboxState;			/* Is this game instance occupying any resources */
	extern bool				IsChainloading;			/* Is the AddonHost chainloading another proxy dll*/
	extern bool				IsImGuiInitialized;		/* Is ImGui currently up and running */

	/* start parameters */
	extern bool				IsDeveloperMode;		/* Is the AddonHost running in developer mode */
	extern bool				IsConsoleEnabled;		/* Is the console window enabled */
	extern bool				IsVanilla;				/* Is the AddonHost running in vanilla mode and should not load any mods */
	extern bool				IsMumbleDisabled;		/* Is Mumble intentionally disabled */

	void Initialize();
}

#endif
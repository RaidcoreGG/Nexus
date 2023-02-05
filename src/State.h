#ifndef STATE_H
#define STATE_H

enum class ggState
{
	NONE,
	LOAD,									/* AddonHost is loading */
	UI_READY,								/* AddonHost is ready to initialise UI */
	ADDONS_LOAD,							/* AddonHost is loading addons*/
	ADDONS_READY,							/* AddonHost has loaded the addons */
	ADDONS_SHUTDOWN,						/* AddonHost has loaded the addons */
	SHUTDOWN								/* AddonHost has been shut down */
};

enum class EDxState
{
	NONE,
	DIRECTX_LOAD,							/* Is loading directx (system/chainload) */
	DIRECTX_READY,							/* Has loaded directx */
	DIRECTX_HOOKED,							/* Has installed directx hooks */
};

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

/* Namespace for global state variables */
namespace State
{
	/* internal states */
	extern ggState		AddonHost;				/* AddonHost state variable */
	extern EDxState		Directx;				/* DirectX state variable */
	extern EEntryMethod	EntryMethod;			/* How was the host initialized */
	extern bool			IsChainloading;			/* Is the AddonHost chainloading another proxy dll*/
	extern bool			IsImGuiInitialized;		/* Is ImGui currently up and running */

	/* start parameters */
	extern bool			IsDeveloperMode;		/* Is the AddonHost running in developer mode */
	extern bool			IsConsoleEnabled;		/* Is the console window enabled */
	extern bool			IsVanilla;				/* Is the AddonHost running in vanilla mode and should not load any mods */
	extern bool			IsMumbleDisabled;		/* Is Mumble intentionally disabled */

	void Initialize();
}

#endif
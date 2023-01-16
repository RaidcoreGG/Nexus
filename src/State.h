#ifndef STATE_H
#define STATE_H

/* Namespace for global state variables */

enum class ggState
{
	NONE,
	LOAD,									/* AddonHost is loading */
	READY,									/* AddonHost is ready to initialise UI */
	ADDONS_LOAD,							/* AddonHost is loading addons*/
	ADDONS_READY,							/* AddonHost has loaded the addons */
	SHUTDOWN								/* AddonHost has been shut down */
};

enum class DxState
{
	NONE,
	DIRECTX_LOAD,							/* Is loading directx (system/chainload) */
	DIRECTX_READY,							/* Has loaded directx */
	DIRECTX_HOOKED,							/* Has installed directx hooks */
};

enum class LoadType
{
	NONE,
	DX9_PROXY,
	DX9_CHAINLOAD,
	DX11_PROXY,
	DX11_CHAINLOAD,
	DXGI_PROXY,
	DXGI_CHAINLOAD
};

namespace State
{
	/* internal states */
	extern ggState  AddonHost;				/* AddonHost state variable */
	extern DxState	Directx;				/* DirectX state variable */
	extern bool		IsChainloading;			/* Is the AddonHost chainloading another proxy dll*/
	extern bool		IsImGuiInitialized;		/* Is ImGui currently up and running */

	/* start parameters */
	extern bool		IsDeveloperMode;		/* Is the AddonHost running in developer mode */
	extern bool		IsConsoleEnabled;		/* Is the console window enabled */
	extern bool		IsVanilla;				/* Is the AddonHost running in vanilla mode and should not load any mods */
	extern bool		IsMumbleDisabled;		/* Is Mumble intentionally disabled */
}

#endif
#ifndef STATE_H
#define STATE_H

/* Namespace for global state variables */
namespace State
{
	extern bool IsShutdown;					/* Has the AddonHost been shut down */
	extern bool IsDxLoaded;					/* Has the directx proxy dll been loaded */
	extern bool IsDxCreated;				/* Has Dx::CreateDevice been called */
	extern bool IsChainloading;				/* Is the AddonHost chainloading another proxy dll b*/
	extern bool IsImGuiInitialized;			/* Is ImGui currently up and running */
	extern bool IsImGuiInitializable;		/* Is ImGui allowed to initialise (arbitrary arcdps delay / resize hook) */
	extern bool IsAddonLibraryInitialized;	/* Have the mods been loaded */

	/* start parameters */
	extern bool IsDeveloperMode;			/* Is the AddonHost running in developer mode */
	extern bool IsVanilla;					/* Is the AddonHost running in vanilla mode and should not load any mods */
	extern bool IsConsoleEnabled;			/* Is the console window enabled */
}

#endif
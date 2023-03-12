#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "AddonAPI.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

typedef void (*ADDON_LOAD)(AddonAPI aHostApi);
typedef void (*ADDON_UNLOAD)();
typedef void (*ADDON_RENDER)();
typedef bool (*ADDON_WNDPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct AddonDefinition
{
	/* required */
	signed int      Signature;      /* Raidcore Addon ID, set to random negative integer if not on Raidcore */
	const char*     Name;           /* Name of the addon as shown in the library */
	const char*     Version;        /* Leave as `__DATE__ L" " __TIME__` to maintain consistency */
	const char*     Author;         /* Author of the addon */
	const char*     Description;    /* Short description */
	ADDON_LOAD      Load;           /* Pointer to Load Function of the addon */
	ADDON_UNLOAD    Unload;         /* Pointer to Unload Function of the addon */
	EAddonFlags     Flags;          /* Information about the addon */

	/* optional */
	ADDON_RENDER    Render;         /* Present callback to render imgui */
	ADDON_WNDPROC   WndProc;        /* WndProc callback for custom implementations beyond keybinds, return true if processed by addon, false if passed through*/

	/* update fallback */
	EUpdateProvider Provider;       /* What platform is the the addon hosted on */
	const char*     UpdateLink;     /* Link to the update resource */

	/* internal */
	bool HasMinimumRequirements();
	bool HasFlag(EAddonFlags aAddonFlag);
};

#endif
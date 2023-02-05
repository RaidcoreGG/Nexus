#include <Windows.h>

#include "Paths.h"

#include "core.h"

namespace Path
{
	char D_GW2							[MAX_PATH]{};
	char D_GW2_ADDONS					[MAX_PATH]{};
	char D_GW2_ADDONS_RAIDCORE			[MAX_PATH]{};
	char D_GW2_ADDONS_RAIDCORE_FONTS	[MAX_PATH]{};
	char D_GW2_ADDONS_RAIDCORE_LOCALES	[MAX_PATH]{};

	char F_HOST_DLL						[MAX_PATH]{};
	char F_TEMP_DLL						[MAX_PATH]{};
	char F_SYSTEM_DLL					[MAX_PATH]{};
	char F_CHAINLOAD_DLL				[MAX_PATH]{};

	char F_LOG							[MAX_PATH]{};
	char F_KEYBINDS						[MAX_PATH]{};

	void Initialize(HMODULE aBaseModule)
	{
		GetModuleFileNameA(aBaseModule, Path::F_HOST_DLL, MAX_PATH);										/* get self dll path */

		/* directories */
		PathGetDirectoryName(Path::F_HOST_DLL, Path::D_GW2);												/* get current directory */
		PathCopyAndAppend(Path::D_GW2, Path::D_GW2_ADDONS, "addons");										/* get addons path */
		PathCopyAndAppend(Path::D_GW2_ADDONS, Path::D_GW2_ADDONS_RAIDCORE, "Raidcore");						/* get addons Raidcore path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_FONTS, "Fonts");			/* get addons Raidcore path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_LOCALES, "Locales");		/* get addons Raidcore path */

		/* ensure folder tree*/
		CreateDirectoryA(Path::D_GW2_ADDONS_RAIDCORE, nullptr);												/* ensure Raidcore dir */
		CreateDirectoryA(Path::D_GW2_ADDONS_RAIDCORE_FONTS, nullptr);										/* ensure Raidcore/Fonts dir */
		CreateDirectoryA(Path::D_GW2_ADDONS_RAIDCORE_LOCALES, nullptr);										/* ensure Raidcore/Locales dir */

		/* ensure files */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_LOG, "AddonHost.log");						/* get log path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_KEYBINDS, "Keybinds.dat");					/* get keybinds path */

		/* static paths */
		PathCopyAndAppend(Path::D_GW2, Path::F_TEMP_DLL, "d3d11.tmp");										/* get temp dll path */
		PathSystemAppend(Path::F_SYSTEM_DLL, "d3d11.dll");													/* get system dll path */
		PathCopyAndAppend(Path::D_GW2, Path::F_CHAINLOAD_DLL, "d3d11_chainload.dll");						/* get chainload dll path */
	}
}
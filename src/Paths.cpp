#include <Windows.h>

#include "Paths.h"

#include "core.h"

namespace Path
{
	wchar_t D_GW2							[MAX_PATH]{};
	wchar_t D_GW2_ADDONS					[MAX_PATH]{};
	wchar_t D_GW2_ADDONS_RAIDCORE			[MAX_PATH]{};
	wchar_t D_GW2_ADDONS_RAIDCORE_FONTS		[MAX_PATH]{};
	wchar_t D_GW2_ADDONS_RAIDCORE_LOCALES	[MAX_PATH]{};

	wchar_t F_HOST_DLL						[MAX_PATH]{};
	wchar_t F_TEMP_DLL						[MAX_PATH]{};
	wchar_t F_CHAINLOAD_DLL					[MAX_PATH]{};
	wchar_t F_SYSTEM_DLL					[MAX_PATH]{};

	wchar_t F_LOG							[MAX_PATH]{};
	wchar_t F_KEYBINDS_JSON					[MAX_PATH]{};

	void Initialize(HMODULE aBaseModule)
	{
		GetModuleFileNameW(aBaseModule, Path::F_HOST_DLL, MAX_PATH);										/* get self dll path */

		/* directories */
		PathGetDirectoryName(Path::F_HOST_DLL, Path::D_GW2);												/* get current directory */
		PathCopyAndAppend(Path::D_GW2, Path::D_GW2_ADDONS, L"addons");										/* get addons path */
		PathCopyAndAppend(Path::D_GW2_ADDONS, Path::D_GW2_ADDONS_RAIDCORE, L"Raidcore");					/* get addons Raidcore path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_FONTS, L"Fonts");		/* get addons Raidcore path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_LOCALES, L"Locales");	/* get addons Raidcore path */

		/* ensure folder tree*/
		CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE, nullptr);												/* ensure Raidcore dir */
		CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE_FONTS, nullptr);										/* ensure Raidcore/Fonts dir */
		CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE_LOCALES, nullptr);										/* ensure Raidcore/Locales dir */

		/* ensure files */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_LOG, L"AddonHost.log");						/* get log path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_KEYBINDS_JSON, L"keybinds.json");			/* get keybinds path */

		/* static paths */
		PathCopyAndAppend(Path::D_GW2, Path::F_TEMP_DLL, L"d3d11.tmp");										/* get temp dll path */
		PathCopyAndAppend(Path::D_GW2, Path::F_CHAINLOAD_DLL, L"d3d11_chainload.dll");						/* get chainload dll path */
		PathSystemAppend(Path::F_SYSTEM_DLL, L"d3d11.dll");													/* get system dll path */
	}
}
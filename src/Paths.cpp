#include "Paths.h"

namespace Path
{
	char D_GW2							[MAX_PATH]{};
	char D_GW2_ADDONS					[MAX_PATH]{};
	char D_GW2_ADDONS_COMMON			[MAX_PATH]{};
	char D_GW2_ADDONS_COMMON_API_V2		[MAX_PATH]{};
	char D_GW2_ADDONS_NEXUS				[MAX_PATH]{};
	char D_GW2_ADDONS_RAIDCORE_LOCALES	[MAX_PATH]{};

	char F_HOST_DLL						[MAX_PATH]{};
	char F_UPDATE_DLL					[MAX_PATH]{};
	char F_OLD_DLL						[MAX_PATH]{};
	char F_SYSTEM_DLL					[MAX_PATH]{};
	char F_CHAINLOAD_DLL				[MAX_PATH]{};

	char F_LOG							[MAX_PATH]{};
	char F_KEYBINDS						[MAX_PATH]{};
	char F_FONT							[MAX_PATH]{};
	char F_SETTINGS						[MAX_PATH]{};
	char F_APIKEYS						[MAX_PATH]{};

	void Initialize(HMODULE aBaseModule)
	{
		GetModuleFileNameA(aBaseModule, Path::F_HOST_DLL, MAX_PATH);										/* get self dll path */

		/* directories */
		PathGetDirectoryName(Path::F_HOST_DLL, Path::D_GW2);												/* get current directory */
		PathCopyAndAppend(Path::D_GW2, Path::D_GW2_ADDONS, "addons");										/* get addons path */
		PathCopyAndAppend(Path::D_GW2_ADDONS, Path::D_GW2_ADDONS_COMMON, "common");							/* get addons/common path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_COMMON, Path::D_GW2_ADDONS_COMMON_API_V2, "api/v2");			/* get addons/common/api/v2 path */
		PathCopyAndAppend(Path::D_GW2_ADDONS, Path::D_GW2_ADDONS_NEXUS, "Nexus");							/* get addons/Nexus path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_NEXUS, Path::D_GW2_ADDONS_RAIDCORE_LOCALES, "Locales");		/* get addons/Nexus/Locales path */

		/* ensure folder tree*/
		CreateDirectoryA(Path::D_GW2_ADDONS, nullptr);														/* ensure addons dir */
		CreateDirectoryA(Path::D_GW2_ADDONS_COMMON, nullptr);												/* ensure addons/common dir */
		CreateDirectoryA(Path::D_GW2_ADDONS_NEXUS, nullptr);												/* ensure addons/Nexus dir */
		CreateDirectoryA(Path::D_GW2_ADDONS_RAIDCORE_LOCALES, nullptr);										/* ensure addons/Nexus/Locales dir */

		/* files */
		PathCopyAndAppend(Path::D_GW2_ADDONS_NEXUS, Path::F_LOG, "Nexus.log");								/* get log path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_NEXUS, Path::F_KEYBINDS, "Keybinds.json");						/* get keybinds path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_NEXUS, Path::F_FONT, "Font.ttf");								/* get font path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_NEXUS, Path::F_SETTINGS, "Settings.json");						/* get settings path */
		PathCopyAndAppend(Path::D_GW2_ADDONS_COMMON, Path::F_APIKEYS, "APIKeys.json");						/* get apikeys path */

		/* static paths */
		PathCopyAndAppend(Path::D_GW2, Path::F_UPDATE_DLL, "d3d11.dll.update");								/* get update dll path */
		PathCopyAndAppend(Path::D_GW2, Path::F_OLD_DLL, "d3d11.dll.old");									/* get old dll path */
		PathSystemAppend(Path::F_SYSTEM_DLL, "d3d11.dll");													/* get system dll path */
		PathCopyAndAppend(Path::D_GW2, Path::F_CHAINLOAD_DLL, "d3d11_chainload.dll");						/* get chainload dll path */
	}

	const char* GetGameDirectory()
	{
		char* str = new char[MAX_PATH]{};
		memcpy(&str[0], &D_GW2[0], strlen(D_GW2));
		return str;
	}

	const char* GetAddonDirectory(const char* aName)
	{
		char* str = new char[MAX_PATH]{};
		size_t idx = strlen(D_GW2_ADDONS);
		memcpy(&str[0], &D_GW2_ADDONS[0], strlen(D_GW2_ADDONS));

		if (strcmp(aName, "") != 0) {
			str[idx] = '\\';
			memcpy(&str[idx + 1], aName, strlen(aName));
		}

		return str;
	}

	const char* GetCommonDirectory()
	{
		char* str = new char[MAX_PATH]{};
		memcpy(&str[0], &D_GW2_ADDONS_COMMON[0], strlen(D_GW2_ADDONS_COMMON));
		return str;
	}
}
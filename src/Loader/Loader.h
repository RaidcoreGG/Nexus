#ifndef LOADER_H
#define LOADER_H

#include "AddonDefinition.h"
#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>

typedef struct arcdps_exports
{
    uint64_t size; // [required]
    uint32_t sig; // [required]
    uint32_t imguivers; // [required]
    const char* out_name; // [required]
    const char* out_build; // [required]
    void* wnd_nofilter;
    void* combat;
    void* imgui;
    void* options_tab;
    void* combat_local;
    void* wnd_filter;
    void* options_windows;
} arcdps_exports;

typedef AddonDefinition*	(*GETADDONDEF)();
typedef void*				(*ARC_GETINITADDR)(char* aArcVersion, ImGuiContext* aImguiContext, void* aID3DPTR, HANDLE aArcDLL, void* aMallocFn, void* aFreeFn, uint32_t aD3DVersion);
typedef unsigned			(*ARC_ADDEXTENSION)(arcdps_exports* aArcExports, unsigned aArcExportsSize, HINSTANCE aModule);
typedef arcdps_exports*     (*ARC_MODINIT)();

namespace Loader
{
	extern std::mutex AddonsMutex;
	extern std::map<std::filesystem::path, AddonDefinition*> AddonDefs;
	extern std::map<std::filesystem::path, HMODULE> AddonModules;
    extern AddonAPI APIDef;

	extern std::thread UpdateThread;

	extern std::set<std::filesystem::path> ExistingLibs;
	extern std::set<std::filesystem::path> Blacklist;

	void Initialize();
	void Shutdown();

	void LoadAddon(std::filesystem::path aPath);
	void UnloadAddon(std::filesystem::path aPath, bool manual = false);

	void Update();
}

#endif
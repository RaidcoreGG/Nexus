#ifndef LOADER_H
#define LOADER_H

#include "AddonDefinition.h"
#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>

struct ArcDPSDefs
{
    uint64_t Size; // [required]
    uint32_t Signature; // [required]
    uint32_t ImGuiVersion; // [required]
    const char* Name; // [required]
    const char* Build; // [required]
    void* WndProc;
    void* Combat;
    void* Present;
    void* OptionsTab;
    void* CombatLocal;
    void* WndProcFiltered;
    void* Windows;
};

typedef AddonDefinition*	(*GETADDONDEF)();
typedef void*				(*ARC_GETINITADDR)(char* aArcVersion, ImGuiContext* aImguiContext, void* aID3DPTR, HANDLE aArcDLL, void* aMallocFn, void* aFreeFn, uint32_t aD3DVersion);
typedef unsigned			(*ARC_ADDEXTENSION)(ArcDPSDefs* aArcExports, unsigned aArcExportsSize, HINSTANCE aModule);
typedef ArcDPSDefs*         (*ARC_MODINIT)();

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
	void UnloadAddon(std::filesystem::path aPath);

	void Update();
}

#endif
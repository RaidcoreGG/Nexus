#ifndef LOADER_ARCDPS_H
#define LOADER_ARCDPS_H

#include <Windows.h>
#include <mutex>
#include <vector>

#include "Core/Addons/Library/LibAddon.h"

namespace ArcDPS
{
	typedef int (*addextension2)(HINSTANCE);
	typedef void (*listextension)(void* callback_fn);

	typedef struct arcdps_exports_t {
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
	} arcdps_exports_t;

	extern std::mutex					Mutex;
	extern HMODULE						ModuleHandle;
	extern bool							IsLoaded;
	extern bool							IsBridgeDeployed;
	extern bool							IsPluginAtlasBuilt;
	extern std::vector<int>				Plugins;

	extern addextension2				exp_addextension2;
	extern listextension				exp_listextension;

	/* Detect if ArcDPS is not loaded as Nexus addon and deploy bridge if found. */
	void Detect();
	/* Write ArcDPS Bridge to disk and queue load. */
	void DeployBridge();
	/* Initialize ArcDPS Bridge. */
	void InitializeBridge(HMODULE aBridgeModule);
	/* Get plugins from arcdps. */
	void GetPlugins();
	/* Callback to receive arcdps plugins. */
	void AddToAtlas(arcdps_exports_t* aArcdpsExports);
	/* Callback to receive arcdps plugins. */
	void AddToAtlasBySig(unsigned int aArcSignature);
	/* Add extension to arcdps. */
	void Add(HMODULE aModule);
}

#endif

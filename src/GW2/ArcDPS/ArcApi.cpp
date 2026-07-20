///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcApi.cpp
/// Description  :  API to call native ArcDPS functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ArcApi.h"

#include "Runtime/Runtime.h"

#include "ArcExtensionDef.h"
#include "Index/Index.h"
#include "Util/DLL.h"

namespace Raidcore::Nexus::GW2
{
	static void s_ReceiveExtension(ArcDPS::ExtensionDefRaw_t* aExtensionDef)
	{
		Runtime::Get().Game().Arcdps().ReceiveExtension(aExtensionDef);
	}

	ArcdpsApi::~ArcdpsApi()
	{
		this->Functions = {};

		if (this->Module)
		{
			FreeLibrary(this->Module);
		}
	}

	bool ArcdpsApi::IsInitialized()
	{
		return this->Module != nullptr;
	}

	void ArcdpsApi::Initialize(HMODULE aArcdpsModule)
	{
		if (!this->BuildFunctionTable(aArcdpsModule))
		{
			throw "Incompatible Module. Missing Functions.";
		}

		char path[MAX_PATH]{};
		GetModuleFileNameA(aArcdpsModule, path, sizeof(path));

		this->Module = LoadLibraryA(path);
	}

	bool ArcdpsApi::TryDetect()
	{
		if (this->IsInitialized())
		{
			return true;
		}

		/* Based on whether Nexus is chainload or primary d3d11, return the other to check against it. */
		std::filesystem::path d3d11 = Index(EPath::D3D11) == Index(EPath::NexusDLL) ? Index(EPath::D3D11Chainload) : Index(EPath::D3D11);

		if (std::filesystem::exists(d3d11))
		{
			HMODULE hModule = GetModuleHandleA(d3d11.string().c_str());

			try
			{
				if (hModule && this->BuildFunctionTable(hModule))
				{
					/* Increment refcount. */
					this->Module = LoadLibraryA(d3d11.string().c_str());
					return true;
				}
			}
			catch (...) { /* nop: D3D11 might just simply be something else. */ }
		}

		std::filesystem::path arc_al = Index(EPath::DIR_ADDONS) / "arcdps/gw2addon_arcdps.dll";

		if (std::filesystem::exists(arc_al))
		{
			HMODULE hModule = GetModuleHandleA(arc_al.string().c_str());

			if (hModule && this->BuildFunctionTable(hModule))
			{
				/* Increment refcount. */
				this->Module = LoadLibraryA(arc_al.string().c_str());
				return true;
			}
		}

		return false;
	}

	std::filesystem::path ArcdpsApi::GetArcIniPath()
	{
		if (this->IsInitialized()) { return std::filesystem::path(); }

		std::filesystem::path path = this->Functions.GetArcIniPath();
		return path;
	}

	void ArcdpsApi::LogToFile(char* aMessage)
	{
		if (this->IsInitialized()) { return; }

		this->Functions.LogToFile(aMessage);
	}

	void ArcdpsApi::LogToWindow(char* aMessage)
	{
		if (this->IsInitialized()) { return; }

		this->Functions.LogToWindow(aMessage);
	}

	ArcDPS::UiFlags_t ArcdpsApi::GetUIFlags()
	{
		if (this->IsInitialized()) { return ArcDPS::UiFlags_t(); }

		return this->Functions.GetUIFlags();
	}

	ArcDPS::Modifiers_t ArcdpsApi::GetModifiers()
	{
		if (this->IsInitialized()) { return ArcDPS::Modifiers_t(); }

		return this->Functions.GetModifiers();
	}

	ArcDPS::EAddExtResult ArcdpsApi::AddExtension(HMODULE aModule)
	{
		if (this->IsInitialized()) { return ArcDPS::EAddExtResult::NotInitialized; }

		return this->Functions.AddExtension(aModule);
	}

	void ArcdpsApi::FreeExtension(uint32_t aSignature)
	{
		if (this->IsInitialized()) { return; }

		this->Functions.FreeExtension(aSignature);
	}

	void ArcdpsApi::PollExtensions()
	{
		if (this->IsInitialized()) { return; }


		this->Functions.ListExtensions(s_ReceiveExtension);
	}

	void ArcdpsApi::ReceiveExtension(ArcDPS::ExtensionDefRaw_t* aExtensionDef)
	{
		if (this->IsInitialized()) { return; }

		throw "Not Implemented";

		/* They need to be added to the loader from here. */
		/* If they are inside /addons, they are probably already tracked. */
		/* If they are outside of /addons though, it's nice to track them. */
	}

	bool ArcdpsApi::BuildFunctionTable(HMODULE aArcdpsModule)
	{
		FuncTable_t funcs{};

		if (DLL::FindFunction(aArcdpsModule, &funcs.GetArcIniPath, "e0") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.LogToFile, "e3") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.LogToWindow, "e8") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.GetUIFlags, "e6") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.GetModifiers, "e7") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.AddExtension, "addextension2") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.FreeExtension, "freeextension2") == false) { return false; }
		if (DLL::FindFunction(aArcdpsModule, &funcs.ListExtensions, "listextension") == false) { return false; }

		/* Store the built function table. */
		this->Functions = funcs;

		return true;
	}
}

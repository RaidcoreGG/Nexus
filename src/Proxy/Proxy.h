///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Proxy.h
/// Description  :  Implementation of proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <string>
#include <windows.h>

#include "Util/Dll.h"

#ifndef PROXY
#define PROXY extern "C"
#endif

#ifndef PROTECT_RECURSE
#define PROTECT_RECURSE(x) ProxyCallScope_t _scope(x)
#endif

///----------------------------------------------------------------------------------------------------
/// ProxyModule_t Struct
///----------------------------------------------------------------------------------------------------
struct ProxyModule_t
{
	bool    Initialized { false };
	HMODULE System      { nullptr };
	HMODULE Chainload   { nullptr };

	///----------------------------------------------------------------------------------------------------
	/// Init:
	/// 	Initializes the proxy module.
	///----------------------------------------------------------------------------------------------------
	inline void Init(const std::string& aModule)
	{
		static HMODULE s_SelfModule = GetCurrentModule();

		if (this->Initialized)
		{
			return;
		}

		this->Initialized = true;

		char sysRoot[MAX_PATH]{};
		GetSystemDirectoryA(sysRoot, MAX_PATH);

		std::filesystem::path systemPath = std::filesystem::path{ sysRoot } / aModule;
		std::filesystem::path chainloadPath = aModule.substr(0, aModule.find_last_of('.')) + "_chainload.dll";

		this->System = LoadLibraryA(systemPath.string().c_str());
		this->Chainload = LoadLibraryA(chainloadPath.string().c_str());

		if (this->Chainload == s_SelfModule)
		{
			this->Chainload = nullptr;
		}
	}

	///----------------------------------------------------------------------------------------------------
	/// GetFunc:
	/// 	Returns the function pointer for the specified function.
	/// 	If in a proxy call, it will return the function pointer from the system module.
	///----------------------------------------------------------------------------------------------------
	template <typename Fn>
	Fn GetFunc(const char* aFunction, bool aInProxyCall)
	{
		HMODULE target = nullptr;

		// recursion protection → system only
		if (aInProxyCall || !this->Chainload)
		{
			target = this->System;
		}
		else
		{
			target = this->Chainload ? this->Chainload : this->System;
		}

		if (!target)
		{
			return nullptr;
		}

		void* addr = GetProcAddress(target, aFunction);

		if (!addr && target == this->Chainload)
		{
			// fallback to system if missing in chain
			addr = GetProcAddress(this->System, aFunction);
		}

		return reinterpret_cast<Fn>(addr);
	}
};

///----------------------------------------------------------------------------------------------------
/// ProxyCallScope_t Struct
/// 	Acquires a proxy call scope, which is used to determine if the current call is a proxy call or not.
///----------------------------------------------------------------------------------------------------
class ProxyCallScope_t
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	ProxyCallScope_t(bool& aFlag) : Flag(aFlag)
	{
		this->Flag = true;
	}

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~ProxyCallScope_t()
	{
		this->Flag = false;
	}

	private:
	bool& Flag;
};

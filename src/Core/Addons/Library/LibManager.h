///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibManager.h
/// Description  :  Manager for available addon libraries.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LIBMANAGER_H
#define LIBMANAGER_H

#include <mutex>
#include <vector>
#include <string>

#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "LibAddon.h"

constexpr const char* CH_LIBRARY = "AddonLibrary";

///----------------------------------------------------------------------------------------------------
/// CLibraryMgr Class
///----------------------------------------------------------------------------------------------------
class CLibraryMgr
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLibraryMgr(CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLibraryMgr();

	///----------------------------------------------------------------------------------------------------
	/// Update:
	/// 	Updates the library addon definitions from the available sources.
	///----------------------------------------------------------------------------------------------------
	void Update();

	///----------------------------------------------------------------------------------------------------
	/// AddSource:
	/// 	Adds a source for library addons definitions.
	///----------------------------------------------------------------------------------------------------
	void AddSource(std::string aURL);

	private:
	CLogApi*                                      Logger;

	std::mutex                                    Mutex;
	std::unordered_map<std::string, CHttpClient*> Sources;
	std::vector<LibraryAddon_t>                   Addons;
};

#endif

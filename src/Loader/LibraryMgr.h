///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibraryMgr.h
/// Description  :  Loader component for managing addon libraries.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LIBRARYMGR_H
#define LIBRARYMGR_H

#include <mutex>
#include <vector>
#include <string>

#include "LibraryAddon.h"
#include "Services/Logging/LogHandler.h"

///----------------------------------------------------------------------------------------------------
/// CLibraryMgr Class
///----------------------------------------------------------------------------------------------------
class CLibraryMgr
{
	public:
	CLibraryMgr(CLogHandler* aLogger);
	~CLibraryMgr();

	void Update();

	void AddSource(const std::string& aURL);

	private:
	CLogHandler*              Logger;

	std::mutex                Mutex;
	std::vector<std::string>  Sources;
	std::vector<LibraryAddon> Addons;
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Migrations.cpp
/// Description  :  Upgrades files left by older versions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Migrations.h"

#include <filesystem>
#include <string>

#include "Host/GW2/Index/Index.h"
#include "Version.h"

/* The version is the build date, so a migration stops compiling from its removal month on. */
#define MIGRATION(aFunction, aRemoveYear, aRemoveMonth)                                              \
	static_assert(V_MAJOR * 12 + V_MINOR < (aRemoveYear) * 12 + (aRemoveMonth),                      \
		"Migration " #aFunction " is due for removal (" #aRemoveYear "-" #aRemoveMonth ").");        \
	aFunction()

namespace Migrations
{
	static void RemoveUnpackedLocales()
	{
		for (const char* lang : { "en", "de", "fr", "es", "cn", "kr", "br", "cz", "it", "pl", "ru" })
		{
			std::error_code ec;
			std::filesystem::remove(Index(EPath::DIR_LOCALES) / (std::string(lang) + "_Main.json"), ec);
		}
	}

	void Run()
	{
		/*        Function               Remove in */
		MIGRATION(RemoveUnpackedLocales, 2027, 3);
	}
}

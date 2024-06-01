///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Library.cpp
/// Description  :  Handles installation of new addons and fetching them from the Raidcore API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Library.h"

#include "Loader.h"
#include "ArcDPS.h"

#include "core.h"
#include "Consts.h"
#include "Shared.h"
#include "Paths.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::string extDll = ".dll";

namespace Loader
{
	namespace Library
	{
		std::mutex					Mutex;
		std::vector<LibraryAddon*>	Addons;

		void Fetch()
		{
			json response = RaidcoreAPI->Get("/addonlibrary");

			if (!response.is_null())
			{
				const std::lock_guard<std::mutex> lock(Mutex);
				Addons.clear();

				for (const auto& addon : response)
				{
					LibraryAddon* newAddon = new LibraryAddon{};
					newAddon->Signature = addon["id"];
					newAddon->Name = addon["name"];
					newAddon->Author = addon["author"];
					newAddon->Description = addon["description"];
					newAddon->Provider = GetProvider(addon["download"]);
					newAddon->DownloadURL = addon["download"];
					if (addon.contains("new") && !addon["new"].is_null())
					{
						newAddon->IsNew = addon["new"];
					}
					if (addon.contains("tos_compliance") && !addon["tos_compliance"].is_null())
					{
						newAddon->ToSComplianceNotice = addon["tos_compliance"];
					}

					Addons.push_back(newAddon);
				}

				std::sort(Addons.begin(), Addons.end(), [](LibraryAddon* a, LibraryAddon* b) {
					if (a->IsNew != b->IsNew)
					{
						return a->IsNew > b->IsNew;
					}
					return a->Name < b->Name;
					});
			}
			else
			{
				LogWarning(CH_LOADER, "Error parsing API response for /addonlibrary.");
			}
		}
	}
}

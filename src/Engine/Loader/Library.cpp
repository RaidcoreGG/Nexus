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

#include "Consts.h"
#include "Core/Context.h"
#include "Engine/Index/Index.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::string extDll = ".dll";

namespace Loader
{
	namespace Library
	{
		std::mutex					Mutex;
		std::vector<LibraryAddon_t*>	Addons;

		void Fetch()
		{
			json response = CContext::GetContext()->GetRaidcoreApi()->Get("/addonlibrary");

			if (!response.is_null())
			{
				const std::lock_guard<std::mutex> lock(Mutex);
				Addons.clear();

				for (const auto& addon : response)
				{
					LibraryAddon_t* newAddon = new LibraryAddon_t{};
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
					if (addon.contains("addon_policy_tier") && !addon["addon_policy_tier"].is_null())
					{
						newAddon->PolicyTier = addon["addon_policy_tier"];
					}

					Addons.push_back(newAddon);
				}

				std::sort(Addons.begin(), Addons.end(), [](LibraryAddon_t* lhs, LibraryAddon_t* rhs)
					{
						return lhs->IsNew > rhs->IsNew ||
							((lhs->IsNew == rhs->IsNew) && lhs->Name < rhs->Name);
					});
			}
			else
			{
				CContext::GetContext()->GetLogger()->Warning(CH_LOADER, "Error parsing API response for /addonlibrary.");
			}
		}
	}
}

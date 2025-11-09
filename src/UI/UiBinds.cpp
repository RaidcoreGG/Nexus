///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiBinds.cpp
/// Description  :  Contains the implementation to display InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UiBinds.h"

#include "Core/Addons/Addon.h"
#include "Core/Context.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "GW2/Inputs/GameBinds/GbConst.h"

CUiBinds::CUiBinds()
{
}

CUiBinds::~CUiBinds()
{
}

std::vector<InputBindCategory_t> CUiBinds::GetInputBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	return this->DisplayInputBinds;
}

std::unordered_map<std::string, InputBindPacked_t> CUiBinds::GetInputBinds(const std::string& aCategory)
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	auto it = std::find_if(this->DisplayInputBinds.begin(), this->DisplayInputBinds.end(), [aCategory](InputBindCategory_t cat) {
		return cat.Name == aCategory;
	});

	if (it != this->DisplayInputBinds.end())
	{
		return it->InputBinds;
	}

	return {};
}

std::vector<GameInputBindCategory_t> CUiBinds::GetGameBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	return this->DisplayGameBinds;
}

void CUiBinds::UpdateDisplayInputBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	this->DisplayInputBinds.clear();

	CContext* ctx = CContext::GetContext();
	CInputBindApi* inputBindApi = ctx->GetInputBindApi();
	CLoader* loader = ctx->GetLoader();

	/* copy of all InputBinds */
	std::map<std::string, IbMapping_t> InputBindRegistry = inputBindApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string owner = "(null)";

		IAddon* iaddon = loader->GetOwner(inputBind.Handler_DownOnlyAsync);

		if (iaddon)
		{
			CAddon* addon = dynamic_cast<CAddon*>(iaddon);
			owner = addon->GetName();
		}
		else
		{
			/* If no owner found, but a pointer is set, we assume Nexus owns it. */
			if (inputBind.Handler_DownOnlyAsync)
			{
				owner = "Nexus";
			}
		}

		auto it = std::find_if(this->DisplayInputBinds.begin(), this->DisplayInputBinds.end(), [owner](InputBindCategory_t category) { return category.Name == owner; });

		if (it == this->DisplayInputBinds.end())
		{
			InputBindCategory_t cat{};
			cat.Name = owner;
			cat.InputBinds[identifier] =
			{
				IBToString(inputBind.Bind, true),
				inputBind
			};
			this->DisplayInputBinds.push_back(cat);
		}
		else
		{
			it->InputBinds[identifier] =
			{
				IBToString(inputBind.Bind, true),
				inputBind
			};
		}
	}

	/* sort input binds. */
	std::sort(this->DisplayInputBinds.begin(), this->DisplayInputBinds.end(), [](InputBindCategory_t& lhs, InputBindCategory_t& rhs)
	{
		// Nexus first
		if (lhs.Name == "Nexus") return true;
		if (rhs.Name == "Nexus") return false;

		// Inactive second
		if (lhs.Name == "((000088))") return true;
		if (rhs.Name == "((000088))") return false;

		/* Rest alphabetical */
		return lhs.Name < rhs.Name;
	});
}

void CUiBinds::UpdateDisplayGameBinds()
{
	const std::lock_guard<std::mutex> lock(this->DisplayBindsMutex);

	this->DisplayGameBinds.clear();

	CContext* ctx = CContext::GetContext();
	CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();

	/* copy of all InputBinds */
	std::unordered_map<EGameBinds, MultiInputBind_t> InputBindRegistry = gameBindsApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string catName = CategoryNameFrom(identifier);

		auto it = std::find_if(this->DisplayGameBinds.begin(), this->DisplayGameBinds.end(), [catName](GameInputBindCategory_t category) { return category.Name == catName; });

		if (it == this->DisplayGameBinds.end())
		{
			GameInputBindCategory_t cat{};
			cat.Name = catName;
			cat.GameInputBinds[identifier] =
			{
				NameFrom(identifier),
				IBToString(inputBind.Primary, true),
				inputBind.Primary,
				IBToString(inputBind.Secondary, true),
				inputBind.Secondary
			};
			this->DisplayGameBinds.push_back(cat);
		}
		else
		{
			it->GameInputBinds[identifier] =
			{
				NameFrom(identifier),
				IBToString(inputBind.Primary, true),
				inputBind.Primary,
				IBToString(inputBind.Secondary, true),
				inputBind.Secondary
			};
		}
	}
}
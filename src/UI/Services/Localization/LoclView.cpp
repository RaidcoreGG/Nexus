///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclView.cpp
/// Description  :  Merged, read-only view over all locale sources, built off the UI thread.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoclView.h"

const LocaleViewLanguage_t* LocaleView_t::Find(std::string_view aIdOrName) const
{
	auto it = this->Languages.find(aIdOrName);

	if (it != this->Languages.end())
	{
		return &it->second;
	}

	for (const auto& [id, language] : this->Languages)
	{
		if (language.DisplayName == aIdOrName)
		{
			return &language;
		}
	}

	return nullptr;
}

std::shared_ptr<const LocaleView_t> BuildLocaleView(std::vector<std::shared_ptr<const LocaleAtlas_t>> aSources)
{
	auto view = std::make_shared<LocaleView_t>();
	view->Sources = std::move(aSources);

	if (view->Sources.empty() || !view->Sources.front())
	{
		return view;
	}

	for (const auto& [id, texts] : *view->Sources.front())
	{
		view->Languages[id].DisplayName = !texts.DisplayName.empty() ? texts.DisplayName : id;
	}

	for (const std::shared_ptr<const LocaleAtlas_t>& source : view->Sources)
	{
		if (!source)
		{
			continue;
		}

		for (const auto& [id, texts] : *source)
		{
			auto langIt = view->Languages.find(id);

			if (langIt == view->Languages.end())
			{
				continue;
			}

			for (const auto& [key, text] : texts.Texts)
			{
				langIt->second.Own.insert_or_assign(std::string_view{ key }, text.c_str());
			}
		}
	}

	auto englishIt = view->Languages.find(std::string_view{ "en" });
	const LocaleViewLanguage_t* english = englishIt != view->Languages.end() ? &englishIt->second : nullptr;

	for (auto& [id, language] : view->Languages)
	{
		language.Lookup = language.Own;

		if (english && english != &language)
		{
			for (const auto& [key, text] : english->Own)
			{
				language.Lookup.emplace(key, text);
			}
		}
	}

	return view;
}

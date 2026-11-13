///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclView.h
/// Description  :  Merged, read-only view over all locale sources, built off the UI thread.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LoclAtlas.h"

///----------------------------------------------------------------------------------------------------
/// LocaleViewLanguage_t Struct
/// 	One language of a view. Points into the view's sources, owns no text.
///----------------------------------------------------------------------------------------------------
struct LocaleViewLanguage_t
{
	std::string                                       DisplayName;

	/* The language's texts from all sources, later sources win. */
	std::unordered_map<std::string_view, const char*> Own;

	/* Own plus the English text of every key the language lacks. */
	std::unordered_map<std::string_view, const char*> Lookup;
};

///----------------------------------------------------------------------------------------------------
/// LocaleView_t Struct
/// 	Immutable once built. Holds its sources, so every text it points to lives as long as the view.
///----------------------------------------------------------------------------------------------------
struct LocaleView_t
{
	std::vector<std::shared_ptr<const LocaleAtlas_t>>        Sources;
	std::map<std::string, LocaleViewLanguage_t, std::less<>> Languages;

	///----------------------------------------------------------------------------------------------------
	/// Find:
	/// 	Returns the language with the given id (e.g. "en") or display name (e.g. "English"), or nullptr.
	///----------------------------------------------------------------------------------------------------
	const LocaleViewLanguage_t* Find(std::string_view aIdOrName) const;
};

///----------------------------------------------------------------------------------------------------
/// BuildLocaleView:
/// 	Merges the sources in order; later sources win. The first source (the locale files) defines which
/// 	languages exist; texts other sources set for further languages are not shown.
///----------------------------------------------------------------------------------------------------
std::shared_ptr<const LocaleView_t> BuildLocaleView(std::vector<std::shared_ptr<const LocaleAtlas_t>> aSources);

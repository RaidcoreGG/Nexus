///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclText.h
/// Description  :  A localizable Nexus display string.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

///----------------------------------------------------------------------------------------------------
/// CText Class
/// 	A display string, identified by its key (e.g. "Common.Accept"). The Lang:: constants are CText.
/// 	Default constructed it is empty: no text.
///----------------------------------------------------------------------------------------------------
class CText
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	constexpr CText() = default;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	constexpr explicit CText(const char* aKey)
		: KeyStr(aKey)
	{}

	///----------------------------------------------------------------------------------------------------
	/// Text:
	/// 	Returns the text in the active language, else English, else the key. "" if empty.
	///----------------------------------------------------------------------------------------------------
	const char* Text() const;

	///----------------------------------------------------------------------------------------------------
	/// Key:
	/// 	Returns the key, e.g. "Common.Accept", for APIs that take keys. nullptr if empty.
	///----------------------------------------------------------------------------------------------------
	constexpr const char* Key() const
	{
		return this->KeyStr;
	}

	constexpr bool IsEmpty() const
	{
		return this->KeyStr == nullptr;
	}

	private:
	const char* KeyStr = nullptr;
};

/* Generated from res/Locales/en/DisplayStrings.json: the Lang::Area::Name constants. Needs CText above. */
#include "LoclTextIds.h"

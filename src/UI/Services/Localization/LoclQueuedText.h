///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclQueuedText.h
/// Description  :  Changes to the locale atlas, applied on the UI thread.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <variant>

///----------------------------------------------------------------------------------------------------
/// QueuedText_t Struct
/// 	A text set through the string API. Owner is the base address of the setting module, or null.
///----------------------------------------------------------------------------------------------------
struct QueuedText_t
{
	std::string Identifier;
	std::string LanguageIdentifier;
	std::string Text;
	void*       Owner = nullptr;
};

///----------------------------------------------------------------------------------------------------
/// QueuedCleanup_t Struct
/// 	Removes every text owned by a module in the address range, e.g. on unload.
///----------------------------------------------------------------------------------------------------
struct QueuedCleanup_t
{
	void* StartAddress;
	void* EndAddress;
};

/* One queue for both, so a reload's texts are applied after the unload's cleanup. */
using LocaleChange_t = std::variant<QueuedText_t, QueuedCleanup_t>;

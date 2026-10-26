///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UrlProtocol.h
/// Description  :  Shared definitions for the nexus:// protocol handler and its consumer in Nexus.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <cctype>
#include <algorithm>
#include <charconv>

#include "thirdparty/nlohmann/json.hpp"

namespace Raidcore::Nexus::UrlProtocol
{
	constexpr const char* MUTEX_QUEUE     = "GW2NEXUS-Mutex-UrlQueue";

	constexpr const char* EVENT_REQUEST   = "GW2NEXUS-Event-UrlRequest";

	constexpr const char* QUEUE_FILENAME  = "pending_installs.json";

	// Must match the TargetName in UrlHandler.vcxproj.
	constexpr const char* HANDLER_FILENAME = "NexusUrl.exe";

	// Max queue size to prevent malicious link spamming.
	constexpr size_t      QUEUE_MAX       = 32;

	struct QueueEntry_t
	{
		uint32_t Id   = 0;
		int64_t  Time = 0; /* Unix timestamp of the request. */
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueueEntry_t, Id, Time)

	template<typename T>
	std::optional<T> ParseInt(std::string_view aStr, int aBase = 10)
	{
		T value {};
		auto [ptr, ec] = std::from_chars(aStr.data(), aStr.data() + aStr.size(), value, aBase);
		// size check to make sure we parsed everything (no trailing junk)
		if (ec != std::errc{} || ptr != aStr.data() + aStr.size()) { return std::nullopt; }
		return value;
	}

	// Parses "nexus://install/<id>" with an optional trailing slash.
	inline std::optional<uint32_t> ParseInstallUrl(std::string_view aUrl)
	{
		constexpr std::string_view prefix = "nexus://install/";

		if (aUrl.size() <= prefix.size()) { return std::nullopt; }

		auto equalsIgnoreCase = [](char a, char b)
		{
			return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
		};

		if (!std::ranges::equal(prefix, aUrl.substr(0, prefix.size()), equalsIgnoreCase)) { return std::nullopt; }

		std::string_view id = aUrl.substr(prefix.size());

		if (id.back() == '/') { id.remove_suffix(1); }

		auto value = ParseInt<uint32_t>(id);

		if (!value || *value == 0) { return std::nullopt; }

		return *value;
	}
}

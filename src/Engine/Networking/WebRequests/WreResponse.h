///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreResponse.h
/// Description  :  Web request response definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef WRERESPONSE_H
#define WRERESPONSE_H

#include <string>
#include <cstdint>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "WreConst.h"

///----------------------------------------------------------------------------------------------------
/// HttpResponse_t Struct
///----------------------------------------------------------------------------------------------------
struct HttpResponse_t
{
	long long   Time       = 0;
	uint32_t    StatusCode = 0;
	std::string Error;
	std::string Content;
	//std::unordered_map<std::string, std::string> Headers;

	///----------------------------------------------------------------------------------------------------
	/// Status:
	/// 	Returns a status string of the response.
	///----------------------------------------------------------------------------------------------------
	inline std::string Status() const
	{
		return std::to_string(this->StatusCode) + " " + StatusCodeToMessage(this->StatusCode);
	}

	///----------------------------------------------------------------------------------------------------
	/// Success:
	/// 	Returns true if the request was successful.
	///----------------------------------------------------------------------------------------------------
	inline bool Success() const
	{
		return this->Error.empty();
	}

	///----------------------------------------------------------------------------------------------------
	/// ContentJSON:
	/// 	Attempts to parse the content to json.
	///----------------------------------------------------------------------------------------------------
	inline json ContentJSON() const
	{
		try
		{
			json j = json::parse(this->Content);
			return j;
		}
		catch(...) {}

		return json{};
	}
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WreConst.h
/// Description  :  Constant data for web requests.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// NormalizeQuery:
/// 	Normalizes a query to be used in filesystem paths.
///----------------------------------------------------------------------------------------------------
std::string NormalizeQuery(std::string aQuery);

///----------------------------------------------------------------------------------------------------
/// StatusCodeToMessage:
/// 	Returns an status message given a status code.
///----------------------------------------------------------------------------------------------------
std::string StatusCodeToMessage(uint32_t aStatusCode);

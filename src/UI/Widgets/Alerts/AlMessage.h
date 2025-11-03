///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AlMessage.h
/// Description  :  Alert message struct defintion.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "AlEnum.h"

///----------------------------------------------------------------------------------------------------
/// AlertMessage_t Struct
///----------------------------------------------------------------------------------------------------
struct AlertMessage_t
{
	EAlertType  Type;
	std::string Message;
	double      StartTime = 0;
};

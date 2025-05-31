///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AlMessage.h
/// Description  :  Alert message struct defintion.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ALMESSAGE_H
#define ALMESSAGE_H

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

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ManagedInputBind.h
/// Description  :  ManagedInputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MANAGEDINPUTBIND_H
#define MANAGEDINPUTBIND_H

#include "InputBind.h"
#include "EInputBindHandlerType.h"

///----------------------------------------------------------------------------------------------------
/// ManagedInputBind Struct
///----------------------------------------------------------------------------------------------------
struct ManagedInputBind
{
	InputBind				Bind;
	EInputBindHandlerType	HandlerType;
	void*					Handler;
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlFuncDefs.h
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Core Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Core
{
	typedef void* (*DATALINK_GETRESOURCE)  (const char* aIdentifier);
	typedef void* (*DATALINK_SHARERESOURCE)(const char* aIdentifier, size_t aResourceSize);
}

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IRefCleaner.cpp
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IRefCleaner.h"

#include "RefCleanerContext.h"

namespace Raidcore::Nexus::Memory
{
	IRefCleaner::IRefCleaner(std::string aComponentName)
	{
		RefCleanerContext::Get()->Register(aComponentName, this);
	}

	IRefCleaner::~IRefCleaner()
	{
		RefCleanerContext::Get()->Deregister(this);
	}
}

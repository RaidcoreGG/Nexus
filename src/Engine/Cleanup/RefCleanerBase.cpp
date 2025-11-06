///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerBase.cpp
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RefCleanerBase.h"

#include "RefCleanerContext.h"

IRefCleaner::IRefCleaner(std::string aComponentName)
{
	CRefCleanerContext::Get()->Register(aComponentName, this);
}

IRefCleaner::~IRefCleaner()
{
	CRefCleanerContext::Get()->Deregister(this);
}

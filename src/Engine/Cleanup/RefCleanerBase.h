///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RefCleanerBase.h
/// Description  :  RefCleaner interface implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef REFCLEANERBASE_H
#define REFCLEANERBASE_H

///----------------------------------------------------------------------------------------------------
/// IRefCleaner Interface Class
///----------------------------------------------------------------------------------------------------
class IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes any reference matching the provided address space.
	///----------------------------------------------------------------------------------------------------
	virtual int CleanupRefs(void* aStartAddress, void* aEndAddress) = 0;
};

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnEntry.h
/// Description  :  Function registry entry struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef FNENTRY_H
#define FNENTRY_H

#include <vector>

///----------------------------------------------------------------------------------------------------
/// FuncEntry_t Struct
///----------------------------------------------------------------------------------------------------
struct FuncEntry_t
{
	int                RefCount;
	void*              ActiveFunction; /* Quick access to this->Functions.first(). */
	std::vector<void*> Functions;
};

#endif

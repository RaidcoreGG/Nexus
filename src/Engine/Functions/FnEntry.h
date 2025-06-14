///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FnEntry.h
/// Description  :  Function registry entry struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef FNENTRY_H
#define FNENTRY_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// FuncEntry_t Struct
///----------------------------------------------------------------------------------------------------
struct FuncEntry_t
{
	int32_t RefCount;
	void*   Function;
};

#endif

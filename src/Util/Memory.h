///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Memory.h
/// Description  :  Contains functions for memory related operations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>

///----------------------------------------------------------------------------------------------------
/// Memory Namespace
///----------------------------------------------------------------------------------------------------
namespace Memory
{
	///----------------------------------------------------------------------------------------------------
	/// FollowJmpChain:
	/// 	Follows the jmp chain of a pointer to get the address at the end of it.
	///----------------------------------------------------------------------------------------------------
	PBYTE FollowJmpChain(PBYTE aPointer);
}

#endif

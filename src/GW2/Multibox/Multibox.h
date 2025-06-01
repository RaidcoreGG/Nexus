///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Multibox.h
/// Description  :  Provides functions to enable multiboxing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MULTIBOX_H
#define MULTIBOX_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EMultiboxState Enumeration
///----------------------------------------------------------------------------------------------------
enum class EMultiboxState : uint32_t
{
	NONE           = 0,
	ARCHIVE_SHARED = 1 << 0,
	LOCAL_SHARED   = 1 << 1,
	MUTEX_CLOSED   = 1 << 2,
	READY = ARCHIVE_SHARED | LOCAL_SHARED | MUTEX_CLOSED
};

///----------------------------------------------------------------------------------------------------
/// Multibox Namespace
///----------------------------------------------------------------------------------------------------
namespace Multibox
{
	///----------------------------------------------------------------------------------------------------
	/// GetState:
	/// 	Returns the current state of multiboxing compatibility.
	///----------------------------------------------------------------------------------------------------
	EMultiboxState GetState();

	///----------------------------------------------------------------------------------------------------
	/// KillMutex:
	/// 	Kills the mutex that prevents the game client from opening multiple times.
	///----------------------------------------------------------------------------------------------------
	void KillMutex();
}

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Multibox.h
/// Description  :  Provides functions to enable multiboxing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MULTIBOX_H
#define MULTIBOX_H

///----------------------------------------------------------------------------------------------------
/// EMultiboxState Enumeration
///----------------------------------------------------------------------------------------------------
enum class EMultiboxState
{
	NONE = 0,
	ARCHIVE_SHARED = 1,
	LOCAL_SHARED = 2,
	MUTEX_CLOSED = 4,
	READY = ARCHIVE_SHARED | LOCAL_SHARED | MUTEX_CLOSED
};

EMultiboxState operator|(EMultiboxState lhs, EMultiboxState rhs);
EMultiboxState operator&(EMultiboxState lhs, EMultiboxState rhs);
EMultiboxState operator|=(EMultiboxState& lhs, EMultiboxState rhs);

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
	/// ShareArchive:
	/// 	Shares the game archive that prevents the game client from opening multiple times.
	///----------------------------------------------------------------------------------------------------
	void ShareArchive();

	///----------------------------------------------------------------------------------------------------
	/// ShareLocal:
	/// 	Shares the local archive that prevents the game client from opening multiple times.
	///----------------------------------------------------------------------------------------------------
	void ShareLocal();

	///----------------------------------------------------------------------------------------------------
	/// KillMutex:
	/// 	Kills the mutex that prevents the game client from opening multiple times.
	///----------------------------------------------------------------------------------------------------
	void KillMutex();
}

#endif

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbEnum.h
/// Description  :  InputBind enums.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBENUM_H
#define IBENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// EInputDevice Enumeration
///----------------------------------------------------------------------------------------------------
enum class EInputDevice : uint32_t
{
	None,
	Keyboard,
	Mouse
};

///----------------------------------------------------------------------------------------------------
/// EIbHandlerType Enumeration
///----------------------------------------------------------------------------------------------------
enum class EIbHandlerType : uint32_t
{
	None,
	DownAsync,        /* Type 1 */
	DownReleaseAsync, /* Type 2 */
	DownRelease       /* Type 3 */
};

#endif

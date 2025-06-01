///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlEnum.h
/// Description  :  Enumerations for DataLinks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DLENUM_H
#define DLENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// ELinkedResourceType Enumeration
///----------------------------------------------------------------------------------------------------
enum class ELinkedResourceType : uint32_t
{
	None,
	Public,
	Internal
};

#endif

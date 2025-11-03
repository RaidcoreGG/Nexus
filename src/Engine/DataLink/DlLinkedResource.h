///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlLinkedResource.h
/// Description  :  Contains the linked resources data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <string>

#include "DlEnum.h"

///----------------------------------------------------------------------------------------------------
/// LinkedResource_t Struct
///----------------------------------------------------------------------------------------------------
struct LinkedResource_t
{
	ELinkedResourceType Type;           /* The type of the resource. Public or Internal. */
	HANDLE              Handle;         /* The handle of the resource.                   */
	void*               Pointer;        /* The pointer to the resource.                  */
	size_t              Size;           /* The size of the resource.                     */
	std::string         UnderlyingName; /* The real name of the memory mapped file.      */
};

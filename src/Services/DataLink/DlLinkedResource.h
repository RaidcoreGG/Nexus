///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlLinkedResource.h
/// Description  :  Contains the Texture data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef DLLINKEDRESOURCE_H
#define DLLINKEDRESOURCE_H

#include <windows.h>
#include <string>
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

///----------------------------------------------------------------------------------------------------
/// LinkedResource Struct
///----------------------------------------------------------------------------------------------------
struct LinkedResource
{
	ELinkedResourceType Type;           /* The type of the resource. Public or Internal. */
	HANDLE              Handle;         /* The handle of the resource. */
	void*               Pointer;        /* The pointer to the resource. */
	size_t              Size;           /* The size of the resource. */
	std::string         UnderlyingName; /* The real name of the memory mapped file.*/
};

#endif

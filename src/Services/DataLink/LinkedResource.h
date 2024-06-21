///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LinkedResource.h
/// Description  :  Contains the Texture data struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LINKEDRESOURCE_H
#define LINKEDRESOURCE_H

#include <Windows.h>
#include <string>

///----------------------------------------------------------------------------------------------------
/// ELinkedResourceType enum
///----------------------------------------------------------------------------------------------------
enum class ELinkedResourceType
{
	None,
	Public,
	Internal
};

///----------------------------------------------------------------------------------------------------
/// LinkedResource data struct
///----------------------------------------------------------------------------------------------------
struct LinkedResource
{
	ELinkedResourceType		Type;				/* The type of the resource. Public or Internal. */
	HANDLE					Handle;				/* The handle of the resource. */
	void*					Pointer;			/* The pointer to the resource. */
	size_t					Size;				/* The size of the resource. */
	std::string				UnderlyingName;		/* The real name of the memory mapped file.*/
};

#endif

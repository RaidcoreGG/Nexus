#ifndef LINKEDRESOURCE_H
#define LINKEDRESOURCE_H

#include <Windows.h>

/* A structure holding information about an allocated resource. */
struct LinkedResource
{
	HANDLE	Handle;		/* The handle of the resource. */
	void*	Pointer;	/* The pointer to the resource. */
	size_t	Size;		/* The size of the resource. */
};

#endif
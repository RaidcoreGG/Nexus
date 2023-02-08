#ifndef LINKEDRESOURCE_H
#define LINKEDRESOURCE_H

#include <Windows.h>

struct LinkedResource
{
	HANDLE	Handle;
	void*	Pointer;
	size_t	Size;
};

#endif
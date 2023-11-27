#ifndef MUMBLE_LINKEDMEM_H
#define MUMBLE_LINKEDMEM_H

#include "types.h"
#include "Context.h"

namespace Mumble
{
	struct LinkedMem
	{
		unsigned UIVersion;
		unsigned UITick;
		Vector3 AvatarPosition;
		Vector3 AvatarFront;
		Vector3 AvatarTop;
		wchar_t Name[256];
		Vector3 CameraPosition;
		Vector3 CameraFront;
		Vector3 CameraTop;
		wchar_t Identity[256];
		unsigned ContextLength;
		Context Context;
		wchar_t Description[2048];
	};
}

#endif
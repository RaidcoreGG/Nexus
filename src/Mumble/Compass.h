#ifndef MUMBLE_COMPASS_H
#define MUMBLE_COMPASS_H

#include "types.h"

namespace Mumble
{
	struct Compass
	{
		unsigned short	Width;
		unsigned short	Height;
		float			Rotation; // radians
		Vector2			PlayerPosition; // continent
		Vector2			Center; // continent
		float			Scale;
	};
}

#endif COMPASS_H
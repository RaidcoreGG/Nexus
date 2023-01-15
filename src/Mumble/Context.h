#ifndef MUMBLE_CONTEXT_H
#define MUMBLE_CONTEXT_H

#include "EMapType.h"
#include "Compass.h"
#include "EMountIndex.h"

namespace Mumble
{
	struct Context
	{
		unsigned char ServerAddress[28]; // contains sockaddr_in or sockaddr_in6
		unsigned MapID;
		EMapType MapType;
		unsigned ShardID;
		unsigned InstanceID;
		unsigned BuildID;

		/* data beyond this point is not necessary for identification */
		unsigned IsMapOpen : 1;
		unsigned IsCompassTopRight : 1;
		unsigned IsCompassRotating : 1;
		unsigned IsGameFocused : 1;
		unsigned IsCompetitive : 1;
		unsigned IsTextboxFocused : 1;
		unsigned IsInCombat : 1;
		// unsigned UNUSED1			: 1;
		Compass Compass;
		unsigned ProcessID;
		EMountIndex MountIndex;
	};
}

#endif
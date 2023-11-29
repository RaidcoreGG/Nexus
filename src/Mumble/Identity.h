#ifndef MUMBLE_IDENTITY_H
#define MUMBLE_IDENTITY_H

#include <string>

namespace Mumble
{
	struct Identity
	{
		char		Name[20];
		unsigned	Profession;
		unsigned	Specialization;
		unsigned	Race;
		unsigned	MapID;
		unsigned	WorldID;
		unsigned	TeamColorID;
		bool		IsCommander;
		float		FOV;
		unsigned	UISize;
	};

	bool operator==(const Identity& lhs, const Identity& rhs);

	bool operator!=(const Identity& lhs, const Identity& rhs);
}

#endif
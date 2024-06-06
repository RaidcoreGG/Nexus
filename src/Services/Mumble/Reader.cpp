///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Reader.cpp
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Services/Mumble/Reader.h"

#include "Shared.h"
#include "State.h"

#include "Events/EventHandler.h"
#include "DataLink/DataLink.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Mumble
{
	bool operator==(const Identity& lhs, const Identity& rhs)
	{
		if (strcmp(lhs.Name, rhs.Name) == 0 &&
			lhs.Profession == rhs.Profession &&
			lhs.Specialization == rhs.Specialization &&
			lhs.Race == rhs.Race &&
			lhs.MapID == rhs.MapID &&
			lhs.WorldID == rhs.WorldID &&
			lhs.TeamColorID == rhs.TeamColorID &&
			lhs.IsCommander == rhs.IsCommander &&
			lhs.FOV == rhs.FOV &&
			lhs.UISize == rhs.UISize)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Identity& lhs, const Identity& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const Vector2& lhs, const Vector2& rhs)
	{
		if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
			trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y))
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Vector2& lhs, const Vector2& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const Vector3& lhs, const Vector3& rhs)
	{
		if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
			trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y) &&
			trunc(1000. * lhs.Z) == trunc(1000. * rhs.Z))
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Vector3& lhs, const Vector3& rhs)
	{
		return !(lhs == rhs);
	}

	float GetScalingFactor(EUIScale aSize)
	{
		switch (aSize)
		{
		case EUIScale::Small: return SC_SMALL;	// Small
		default:
		case EUIScale::Normal: return SC_NORMAL;	// Normal
		case EUIScale::Large: return SC_LARGE;	// Large
		case EUIScale::Larger: return SC_LARGER;	// Larger
		}
	}
}

CMumbleReader::CMumbleReader(std::string aMumbleName)
{
	this->Name = aMumbleName;

	/* share the linked mem regardless whether it's disabled, for dependant addons */
	MumbleLink = (Mumble::Data*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(Mumble::Data), MumbleLinkName.c_str());
	NexusLink = (NexusLinkData*)DataLink::ShareResource(DL_NEXUS_LINK, sizeof(NexusLinkData));

	if (aMumbleName == "0")
	{
		this->IsMumbleDisabled = true;
	}
	else
	{
		this->Thread = std::thread(&CMumbleReader::Advance, this);
		this->Thread.detach();
	}
}

void CMumbleReader::Advance()
{
	while (State::Nexus < ENexusState::SHUTTING_DOWN)
	{
		IsGameplay		= PreviousTick != MumbleLink->UITick || (PreviousFrameCounter == FrameCounter && IsGameplay);
		IsMoving		= PreviousAvatarPosition != MumbleLink->AvatarPosition;
		IsCameraMoving	= PreviousCameraFront != MumbleLink->CameraFront;

		this->PreviousFrameCounter		= FrameCounter;
		this->PreviousTick				= MumbleLink->UITick;
		this->PreviousAvatarPosition	= MumbleLink->AvatarPosition;
		this->PreviousCameraFront		= MumbleLink->CameraFront;

		if (MumbleLink->Identity[0])
		{
			/* cache identity */
			this->PreviousIdentity = *MumbleIdentity;

			try
			{
				/* parse and assign current identity */
				json j = json::parse(MumbleLink->Identity);
				strcpy(MumbleIdentity->Name, j["name"].get<std::string>().c_str());
				j["profession"].get_to(MumbleIdentity->Profession);
				j["spec"].get_to(MumbleIdentity->Specialization);
				j["race"].get_to(MumbleIdentity->Race);
				j["map_id"].get_to(MumbleIdentity->MapID);
				j["world_id"].get_to(MumbleIdentity->WorldID);
				j["team_color_id"].get_to(MumbleIdentity->TeamColorID);
				j["commander"].get_to(MumbleIdentity->IsCommander);
				j["fov"].get_to(MumbleIdentity->FOV);
				j["uisz"].get_to(MumbleIdentity->UISize);
			}
			catch (json::parse_error& ex)
			{
				Log(CH_MUMBLE_READER, "MumbleLink could not be parsed. Parse Error: %s", ex.what());
			}
			catch (json::type_error& ex)
			{
				Log(CH_MUMBLE_READER, "MumbleLink could not be parsed. Type Error: %s", ex.what());
			}
			catch (...)
			{
				Log(CH_MUMBLE_READER, "MumbleLink could not be parsed. Unknown Error.");
			}

			/* notify (also notifies the GUI to update its scaling factor) */
			if (*MumbleIdentity != this->PreviousIdentity)
			{
				Events::Raise("EV_MUMBLE_IDENTITY_UPDATED", MumbleIdentity);
			}
		}

		Sleep(50);
	}
}

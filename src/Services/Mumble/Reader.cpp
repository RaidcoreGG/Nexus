///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Reader.cpp
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Services/Mumble/Reader.h"

#include <map>
#include <string>
#include <Windows.h>

#include "Shared.h"
#include "State.h"
#include "Renderer.h"
#include "Consts.h"

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

	bool IsRunning = false;

	std::thread UpdateIdentityThread;
	std::thread UpdateStateThread;

	/* Helpers for state vars */
	static unsigned prevTick = 0;
	static Vector3 prevAvPos{};
	static Vector3 prevCamFront{};
	static Identity prevIdentity{};
	static long long prevFrameCounter = 0;

	void Initialize()
	{
		IsRunning = true;

		UpdateIdentityThread = std::thread(UpdateIdentityLoop);
		UpdateIdentityThread.detach();
		UpdateStateThread = std::thread(UpdateStateLoop);
		UpdateStateThread.detach();

		if (MumbleLinkName == "0")
		{
			State::IsMumbleDisabled = true;
		}
		MumbleLink = (Mumble::Data*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(Mumble::Data), MumbleLinkName.c_str());
		NexusLink = (NexusLinkData*)DataLink::ShareResource(DL_NEXUS_LINK, sizeof(NexusLinkData));
	}

	void Shutdown()
	{
		if (State::Nexus == ENexusState::SHUTTING_DOWN)
		{
			IsRunning = false;
		}
	}

	void UpdateIdentityLoop()
	{
		for (;;)
		{
			if (!IsRunning) { return; }

			if (MumbleLink != nullptr)
			{
				if (MumbleLink->Identity[0])
				{
					/* cache identity */
					prevIdentity = *MumbleIdentity;

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
						Log(CH_CORE, "MumbleLink could not be parsed. Parse Error: %s", ex.what());
					}
					catch (json::type_error& ex)
					{
						Log(CH_CORE, "MumbleLink could not be parsed. Type Error: %s", ex.what());
					}
					catch (...)
					{
						Log(CH_CORE, "MumbleLink could not be parsed. Unknown Error.");
					}
					
					/* notify (also notifies the GUI to update its scaling factor) */
					if (*MumbleIdentity != prevIdentity)
					{
						Events::Raise(EV_MUMBLE_IDENTITY_UPDATED, MumbleIdentity);
					}
				}
			}

			Sleep(50);
		}
	}

	void UpdateStateLoop()
	{
		for (;;)
		{
			if (!IsRunning) { return; }

			if (MumbleLink != nullptr)
			{
				IsGameplay = prevTick != MumbleLink->UITick || (prevFrameCounter == FrameCounter && IsGameplay);
				IsMoving = prevAvPos != MumbleLink->AvatarPosition;
				IsCameraMoving = prevCamFront != MumbleLink->CameraFront;

				prevFrameCounter = FrameCounter;
				prevTick = MumbleLink->UITick;
				prevAvPos = MumbleLink->AvatarPosition;
				prevCamFront = MumbleLink->CameraFront;
			}

			Sleep(100);
		}
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

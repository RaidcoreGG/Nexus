///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Mumble.cpp
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Mumble.h"

#include <map>
#include <string>
#include <Windows.h>

#include "Shared.h"
#include "State.h"
#include "Renderer.h"
#include "Consts.h"

#include "LinkedMem.h"
#include "Events/EventHandler.h"
#include "DataLink/DataLink.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Mumble
{
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

	float GetScalingFactor(unsigned aSize)
	{
		switch (aSize)
		{
		case 0: return SC_SMALL;	// Small
		default:
		case 1: return SC_NORMAL;	// Normal
		case 2: return SC_LARGE;	// Large
		case 3: return SC_LARGER;	// Larger
		}
	}
}

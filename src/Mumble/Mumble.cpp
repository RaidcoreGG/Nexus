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

					/* update ui scaling factor */
					Renderer::Scaling = GetScalingFactor(MumbleIdentity->UISize);

					/* notify */
					if (*MumbleIdentity != prevIdentity)
					{
						Events::Raise(EV_MUMBLE_IDENTITY_UPDATED, MumbleIdentity);
					}
				}
			}

			Sleep(1000);
		}
	}

	void UpdateStateLoop()
	{
		for (;;)
		{
			if (!IsRunning) { return; }

			if (MumbleLink != nullptr)
			{
				IsGameplay = prevTick != MumbleLink->UITick;
				IsMoving = prevAvPos != MumbleLink->AvatarPosition;
				IsCameraMoving = prevCamFront != MumbleLink->CameraFront;

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
			case 0: return Renderer::Scaling = SC_SMALL;;	// Small
			default:
			case 1: return Renderer::Scaling = SC_NORMAL;	// Normal
			case 2: return Renderer::Scaling = SC_LARGE;	// Large
			case 3: return Renderer::Scaling = SC_LARGER;	// Larger
		}
	}
}
#include "Mumble.h"

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
	}

	void Shutdown()
	{
		IsRunning = false;
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
					MumbleIdentity->Name			= j["name"].get<std::string>();
					MumbleIdentity->Profession		= j["profession"].get<unsigned>();
					MumbleIdentity->Specialization	= j["spec"].get<unsigned>();
					MumbleIdentity->Race			= j["race"].get<unsigned>();
					MumbleIdentity->MapID			= j["map_id"].get<unsigned>();
					MumbleIdentity->WorldID			= j["world_id"].get<unsigned>();
					MumbleIdentity->TeamColorID		= j["team_color_id"].get<unsigned>();
					MumbleIdentity->IsCommander		= j["commander"].get<bool>();
					MumbleIdentity->FOV				= j["fov"].get<float>();
					MumbleIdentity->UISize			= j["uisz"].get<unsigned>();

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

			Sleep(50);
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
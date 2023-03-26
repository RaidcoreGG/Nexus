#include "Mumble.h"

using json = nlohmann::json;

namespace Mumble
{
	HANDLE Handle;
	std::thread UpdateIdentityThread;
	std::thread UpdateStateThread;
	bool IsRunning = false;
	unsigned Tick = 0;

	/* Helpers for state vars */
	static unsigned prevTick = 0;
	static Vector3 prevAvPos{};
	static Vector3 prevCamFront{};
	static Identity prevIdentity{};

	LinkedMem* Initialize(const wchar_t* aMumbleName)
	{
		if (wcscmp(aMumbleName, L"0") == 0) { State::IsMumbleDisabled = true; return nullptr; }

		if (Handle && MumbleLink) { return MumbleLink; }

		Handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, aMumbleName);
		if (Handle == 0)
		{
			Handle = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(LinkedMem), aMumbleName);
		}

		if (Handle)
		{
			MumbleLink = (LinkedMem*)MapViewOfFile(Handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));

			IsRunning = true;
			UpdateIdentityThread = std::thread(UpdateIdentity);
			UpdateIdentityThread.detach();
			UpdateStateThread = std::thread(UpdateState);
			UpdateStateThread.detach();

			return MumbleLink;
		}

		return nullptr;
	}

	void Shutdown()
	{
		IsRunning = false;

		if (MumbleLink)
		{
			UnmapViewOfFile((LPVOID)MumbleLink);
			MumbleLink = nullptr;
		}

		if (Handle)
		{
			CloseHandle(Handle);
			Handle = nullptr;
		}
	}

	HANDLE GetHandle()
	{
		return Handle ? Handle : nullptr;
	}

	void UpdateIdentity()
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

	void UpdateState()
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
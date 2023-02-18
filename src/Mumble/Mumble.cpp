#include "Mumble.h"

using json = nlohmann::json;

namespace Mumble
{
	static HANDLE Handle;
	std::thread UpdateThread;
	bool IsRunning = false;

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
			UpdateThread = std::thread(Update);
			UpdateThread.detach();

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

	void Update()
	{
		for (;;)
		{
			if (!IsRunning) { return; }

			if (MumbleLink != nullptr)
			{
				if (MumbleLink->Identity[0])
				{
					Identity prev;

					if (MumbleIdentity == nullptr) { prev = Identity{}; MumbleIdentity = new Identity{}; }
					else { prev = *MumbleIdentity; }

					json j = json::parse(MumbleLink->Identity);

					MumbleIdentity->Name = j["name"].get<std::string>();
					MumbleIdentity->Profession = j["profession"].get<unsigned>();
					MumbleIdentity->Specialization = j["spec"].get<unsigned>();
					MumbleIdentity->Race = j["race"].get<unsigned>();
					MumbleIdentity->MapID = j["map_id"].get<unsigned>();
					MumbleIdentity->WorldID = j["world_id"].get<unsigned>();
					MumbleIdentity->TeamColorID = j["team_color_id"].get<unsigned>();
					MumbleIdentity->IsCommander = j["commander"].get<bool>();
					MumbleIdentity->FOV = j["fov"].get<float>();
					MumbleIdentity->UISize = j["uisz"].get<unsigned>();

					if (State::IsImGuiInitialized)
					{
						switch (MumbleIdentity->UISize)
						{
							case 0: Renderer::Scaling = 0.90f; break; // Small
							case 1: Renderer::Scaling = 1.00f; break; // Normal
							case 2: Renderer::Scaling = 1.10f; break; // Large
							case 3: Renderer::Scaling = 1.20f; break; // Larger
						}
					}

					if (*MumbleIdentity != prev)
					{
						Events::Raise("MUMBLE_IDENTITY_UPDATE", MumbleIdentity);
					}
				}
			}

			Sleep(1000);
		}
	}
}
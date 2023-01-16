#include "Mumble.h"
#include "../Shared.h"
#include "../State.h"
#include "../Renderer.h"
#include "../Events/EventHandler.h"

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

namespace Mumble
{
	static HANDLE Handle;

	LinkedMem* Create(const wchar_t* aMumbleName)
	{
		if (Handle && MumbleLink) { return MumbleLink; }

		//std::wstring mumble_name = GetMumbleName();

		//Handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, mumble_name.c_str());
		Handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, aMumbleName);
		if (Handle == 0)
		{
			Handle = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(LinkedMem), aMumbleName);
		}

		if (Handle)
		{
			MumbleLink = (LinkedMem*)MapViewOfFile(Handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
			return MumbleLink;
		}

		return nullptr;
	}

	void Destroy()
	{
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

	void UpdateIdentity()
	{
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
					EventHandler::RaiseEvent(L"MUMBLE_IDENTITY_UPDATE", MumbleIdentity);
				}
			}
		}
	}
}
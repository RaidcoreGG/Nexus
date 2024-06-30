#pragma once

#include "Loader/EAddonState.h"
#include "Services/Networking/Networking.h"

#include <shared_mutex>
#include <unordered_map>

typedef signed int AddonSignature;

namespace AddonShare {
	struct MemberState {
		Networking::UserId Id;
		std::unordered_map<AddonSignature, EAddonState> Addons;

		inline MemberState(Networking::UserId id, std::unordered_map<AddonSignature, EAddonState>&& addons)
			: Id(id), Addons(addons) { }
	};

	extern std::shared_mutex MemberDataMutex;
	extern std::vector<MemberState> Members;
	/// Addons other members shared with us that are not locally installed and not in the library
	extern std::vector<AddonSignature> SharedAddons;

	void Init();

	void BroadcastAddonStateUpdate(AddonSignature addon, EAddonState state);
	/// Target can be null to broadcast the packet
	void TransmitAddonStates(Networking::UserId* target);
	void RequestAddons();

	void ClearOthers();
	void RemoveSpecificMember(Networking::UserId& member);
}
#include "AddonShare.h"

#include "Shared.h" // Logger instance
#define _LOG(lvl, fmt, ...) Logger->LogMessage(ELogLevel:: lvl, "AddonShare", fmt, __VA_ARGS__)

#include <stdint.h>
#include <thread>

namespace AddonShare {
	using Networking::PacketChecksum;
	using Networking::UserId;

	constexpr AddonSignature AS_PACKET = 1;

	std::shared_mutex MemberDataMutex;
	std::vector<MemberState> Members;
	std::vector<AddonSignature> SharedAddons;

	void PacketHandler(Networking::Packet const* packet);
	void Init()
	{
		Networking::Init();
		Networking::Handlers.emplace(AS_PACKET, PacketHandler);
	}

	enum class PacketType : uint16_t {
		_UNKNOWN = 0,
		IndividualUpdate,
		ListUpdate,
		StatusRequest,
	};

	#pragma pack(push, 1)
	struct PacketHeader : Networking::PacketHeader {
		PacketType Type;
	};
	#pragma pack(pop)

	static_assert(sizeof(PacketHeader) % 4 == 0);

	struct AddonState {
		AddonSignature Signature;
		EAddonState    State;
	};

	static_assert(sizeof(AddonState) % 4 == 0);

	struct StateUpdatePacket {
		PacketHeader   Header = { AS_PACKET, PACKET_LEN(StateUpdatePacket), PacketType::IndividualUpdate };
		UserId         UserId;
		AddonState     Update;
		PacketChecksum CRC;
	};

	struct StatusRequestPacket {
		PacketHeader   Header = { AS_PACKET, PACKET_LEN(StatusRequestPacket), PacketType::StatusRequest };
		PacketChecksum CRC;
	};

	#pragma warning(push)
	#pragma warning(disable : 4200) // nonstandard data[]
	/// Dynamically sized! Must manually initialize all fields and add space for crc!
	struct ListUpdatePacket {
		PacketHeader   Header;
		UserId         UserId;
		AddonState     Updates[];
	};
	#pragma warning(pop)

	constexpr auto PACKET_STATUS_RESPONSE_EMPTY_SIZE = sizeof(ListUpdatePacket) + sizeof(PacketChecksum);

	void BroadcastAddonStateUpdate(AddonSignature addon, EAddonState state)
	{
		StateUpdatePacket p;
		//NOTE(Rennorb): Doesn't matter if its not set; if it isn't, then we don't send the packet either way.
		p.UserId  = Networking::MyUserId;
		p.Update = { addon, state };

		Networking::PrepareAndBroadcastPacket((Networking::Packet*)&p);
	}

	void RequestAddons()
	{
		StatusRequestPacket p;
		Networking::PrepareAndBroadcastPacket((Networking::Packet*)&p);
	}

	void BroadcastAddons()
	{
		std::shared_lock rLock(Loader::AddonsMutex);

		auto packetSize = PACKET_STATUS_RESPONSE_EMPTY_SIZE + sizeof(AddonState) * std::count_if(Loader::Addons.begin(), Loader::Addons.end(), [](Addon* a){ return a->VisibleToSquadMembers; });
		auto p = (ListUpdatePacket*)_malloca(packetSize);
		p->Header = { AS_PACKET, (uint16_t)(packetSize / 4), PacketType::ListUpdate };

		size_t i = 0;
		for(auto addon : Loader::Addons) {
			if(!addon->VisibleToSquadMembers) continue;

			p->Updates[i++] = { addon->Definitions->Signature, addon->State };
		}

		rLock.unlock(); // cannot use scoping here, we don't want to destroy the stackalloced packet

		Networking::PrepareAndBroadcastPacket((Networking::Packet*)p);
	}

	static bool g_goingToBroadcastState = false;

	static void InsertIntoSharedAddonsIfUnknownSig(AddonSignature signature)
	{
		if(!Loader::FindAddonBySig(signature))
			if(std::find(SharedAddons.begin(), SharedAddons.end(), signature) == SharedAddons.end())
				SharedAddons.push_back(signature);
	}

	void PacketHandler(Networking::Packet const* packet)
	{
		auto header = (PacketHeader*)&packet->Header;
		switch(header->Type) {
			case PacketType::IndividualUpdate: {
				auto response = (StateUpdatePacket*)packet;

				std::unique_lock wLock(MemberDataMutex);

				for(auto& member : Members) {
					if(member.Id != response->UserId) continue;

					member.Addons[response->Update.Signature] = response->Update.State;

					InsertIntoSharedAddonsIfUnknownSig(response->Update.Signature);

					goto skip_insertion;
				}

				{
					// we haven't found the member, so we create a new one
					std::unordered_map<AddonSignature, EAddonState> addon_map;
					addon_map.emplace(response->Update.Signature, response->Update.State);

					InsertIntoSharedAddonsIfUnknownSig(response->Update.Signature);

					Members.emplace_back(response->UserId, std::move(addon_map));
				}

				skip_insertion:;
			} break;

			case PacketType::StatusRequest: if(!g_goingToBroadcastState) {
				g_goingToBroadcastState = true;
				// We spawn a thread here to accumulate multiple requests, as it is very likely that we ge multiple requests upon joining a squad.
				//TODO(Rennorb) @perf: Could keep this thread around instead of recreating one every time. Requires more complicated signaling.
				std::thread([]() {
					Sleep(100);

					BroadcastAddons();

					g_goingToBroadcastState = false;
				}).detach();
			} break;

			case PacketType::ListUpdate: {
				auto response = (ListUpdatePacket*)packet;
				auto addonsInPacket = (packet->Header.LengthInU32s * 4 - PACKET_STATUS_RESPONSE_EMPTY_SIZE) / sizeof(AddonState);

				std::unique_lock wLock(MemberDataMutex);
				std::shared_lock rLock(Loader::AddonsMutex);

				for(auto& member : Members) {
					if(member.Id != response->UserId) continue;

					for(size_t i = 0; i < addonsInPacket; i++) {
						auto& addon = response->Updates[i];
						member.Addons[addon.Signature] = addon.State;

						InsertIntoSharedAddonsIfUnknownSig(addon.Signature);
					}

					goto skip_insertion_response;
				}

				{
					// we haven't found the member, so we create a new one
					std::unordered_map<AddonSignature, EAddonState> addon_map;
					addon_map.reserve(addonsInPacket);

					for(size_t i = 0; i < addonsInPacket; i++) {
						auto& addon = response->Updates[i];
						addon_map.try_emplace(addon.Signature, addon.State);

						InsertIntoSharedAddonsIfUnknownSig(addon.Signature);
					}

					Members.emplace_back(response->UserId, std::move(addon_map));
				}

				skip_insertion_response:;
			} break;

			default:
				_LOG(TRACE, "Discarding invalid packet with type %u.", header->Type);
		}
	}

	void ClearOthers()
	{
		std::unique_lock wLock(MemberDataMutex);

		Members.clear();
		SharedAddons.clear();
	}

	void RemoveSpecificMember(Networking::UserId& memberId)
	{
		std::unique_lock wLock(MemberDataMutex);

		for(auto iter = Members.begin(); iter != Members.end(); iter++) {
			if(iter->Id != memberId) continue;

			std::swap(*iter, Members.back());
			Members.pop_back();
			break;
		}
	}
}

#undef _LOG

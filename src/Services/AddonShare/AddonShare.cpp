#include "AddonShare.h"

#include "Shared.h" // Logger instance
#define _LOG(lvl, fmt, ...) Logger->LogMessage(ELogLevel:: lvl, "AddonShare", fmt, __VA_ARGS__)

#include <stdint.h>
#include <thread>

namespace AddonShare {
	using Networking::PacketChecksum;
	using Networking::UserId;
	using Networking::PacketFlags;

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
		PacketHeader   Header = { AS_PACKET, PACKET_LEN(StateUpdatePacket), PacketFlags::ContainsSource, PacketType::IndividualUpdate };
		UserId         UserId; // set by the server
		AddonState     Update;
		PacketChecksum CRC;
	};

	struct StatusRequestPacket {
		PacketHeader   Header = { AS_PACKET, PACKET_LEN(StatusRequestPacket), PacketFlags::ContainsSource, PacketType::StatusRequest };
		UserId         UserId; // set by the server
		PacketChecksum CRC;
	};

	#pragma warning(push)
	#pragma warning(disable : 4200) // nonstandard data[]
	/// Dynamically sized! Must manually initialize all fields and add space for crc!
	struct ListUpdatePacket {
		PacketHeader   Header;
		UserId         UserId; // set by the server
		AddonState     Updates[];
	};
	#pragma warning(pop)

	constexpr auto PACKET_STATUS_RESPONSE_EMPTY_SIZE = sizeof(ListUpdatePacket) + sizeof(PacketChecksum);

	void BroadcastAddonStateUpdate(AddonSignature addon, EAddonState state)
	{
		StateUpdatePacket p;
		p.Update = { addon, state };

		Networking::PrepareAndBroadcastPacket((Networking::Packet*)&p);
	}

	void RequestAddons()
	{
		StatusRequestPacket p;
		Networking::PrepareAndBroadcastPacket((Networking::Packet*)&p);
	}

	/// Target can be null to broadcast the packet
	void TransmitAddonStates(UserId* target)
	{
		std::shared_lock rLock(Loader::AddonsMutex);

		auto packetSize = PACKET_STATUS_RESPONSE_EMPTY_SIZE + sizeof(AddonState) * std::count_if(Loader::Addons.begin(), Loader::Addons.end(), [](Addon* a){ return a->VisibleToSquadMembers; });
		if(target) packetSize += sizeof(UserId);
		//NOTE(Rennorb): Could test for null alloc here, but realistically if we run out of memory what are we going to do? 
		// Might aswell pretend it always works
		auto p = (ListUpdatePacket*)_malloca(packetSize);
		auto flags = PacketFlags::ContainsSource;
		if(target) flags = (PacketFlags)(flags | PacketFlags::ContainsTarget);
		p->Header = { AS_PACKET, (uint8_t)(packetSize / 4), flags, PacketType::ListUpdate };

		size_t i = 0;
		for(auto addon : Loader::Addons) {
			// sometimes the deffinition is 0
			if(!addon->VisibleToSquadMembers || !addon->Definitions) continue;

			p->Updates[i++] = { addon->Definitions->Signature, addon->State };
		}

		rLock.unlock(); // cannot use scoping here, we don't want to destroy the stackalloced packet

		if(target) *(UserId*)(p->Updates + i) = *target;

		Networking::PrepareAndBroadcastPacket((Networking::Packet*)p);
		_freea(p);
	}


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
			case PacketType::IndividualUpdate: if(header->Flags == PacketFlags::ContainsSource) {
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
				return;
			} break;

			case PacketType::StatusRequest: if(header->Flags & PacketFlags::ContainsSource) {
				enum ReportTarget {
					None = 0,
					Single,
					Broadcast,
				};
				static ReportTarget s_reportTarget = None;
				static UserId s_reportTo;
				
				if(s_reportTarget == None) {
					s_reportTarget = Single;
					s_reportTo = ((StatusRequestPacket*)packet)->UserId;
					// We spawn a thread here to accumulate multiple requests, as it is very likely that we ge multiple requests upon merging a squad.
					//TODO(Rennorb) @perf: Could keep this thread around instead of recreating one every time. Requires more complicated signaling.
					std::thread([]() {
						Sleep(100);

						TransmitAddonStates(s_reportTarget == Single ? &s_reportTo : 0);

						s_reportTarget = None;
					}).detach();
				}
				else {
					// if we get multiple requests we just broadcast
					s_reportTarget = Broadcast;
				}
				return;
			} break;

			case PacketType::ListUpdate: if(header->Flags & PacketFlags::ContainsSource) {
				auto response = (ListUpdatePacket*)packet;
				auto addonsInPacket = (header->LengthInU32s * 4 - PACKET_STATUS_RESPONSE_EMPTY_SIZE) / sizeof(AddonState);
				static_assert(sizeof(UserId) % sizeof(AddonState) == 0);
				if(header->Flags & PacketFlags::ContainsTarget) addonsInPacket -= sizeof(UserId) / sizeof(AddonState);

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
				return;
			} break;
		}
		_LOG(TRACE, FORMAT_DISCARD_MESSAGE(packet));
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
			return;
		}

		_LOG(TRACE, "Failed to find member %08x%08x%08x%08x to remove", SPLIT_USER_ID(memberId));
	}
}

#undef _LOG

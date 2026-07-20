///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblReader.h
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "thirdparty/mumble/Mumble.h"

#include "Core/NexusLink.h"
#include "Core/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "Core/Logging/LogApi.h"

constexpr const char* DL_MUMBLE_LINK = "DL_MUMBLE_LINK";
constexpr const char* DL_MUMBLE_LINK_IDENTITY = "DL_MUMBLE_LINK_IDENTITY";
constexpr const char* EV_MUMBLE_IDENTITY_UPDATED = "EV_MUMBLE_IDENTITY_UPDATED";

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	constexpr const char* CH_MUMBLE_READER = "MumbleReader";
	///----------------------------------------------------------------------------------------------------
	/// MumbleReader Class
	///----------------------------------------------------------------------------------------------------
	class MumbleReader
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		MumbleReader(Core::DataLinkApi& aDataLink, Host::EventApi& aEventApi, Core::LogApi& aLogger);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~MumbleReader();

		///----------------------------------------------------------------------------------------------------
		/// GetName:
		/// 	Returns the name of the MumbleLink file.
		///----------------------------------------------------------------------------------------------------
		std::string GetName();

		///----------------------------------------------------------------------------------------------------
		/// IsDisabled:
		/// 	Returns whether the MumbleLink API is explicitly disabled.
		///----------------------------------------------------------------------------------------------------
		bool IsDisabled() const;

		///----------------------------------------------------------------------------------------------------
		/// GetMumbleData:
		/// 	Returns a pointer to the mumble link data.
		///----------------------------------------------------------------------------------------------------
		Mumble::Data* GetMumbleData() const;

		///----------------------------------------------------------------------------------------------------
		/// GetMumbleIdentity:
		/// 	Returns a pointer to the parsed mumble identity data.
		///----------------------------------------------------------------------------------------------------
		Mumble::Identity* GetMumbleIdentity() const;

		///----------------------------------------------------------------------------------------------------
		/// GetNexusLink:
		/// 	Returns a pointer to the nexus link data.
		///----------------------------------------------------------------------------------------------------
		NexusLinkData_t* GetNexusLink() const;

		private:
		Core::DataLinkApi& DataLinkApi;
		Host::EventApi& EventApi;
		Core::LogApi& Logger;

		std::string       Name;
		Mumble::Data*     MumbleLink = nullptr;
		Mumble::Identity* MumbleIdentity = nullptr;
		NexusLinkData_t*  NexusLink = nullptr;

		unsigned          PreviousTick = 0;
		Mumble::Vector3   PreviousAvatarPosition = {};
		Mumble::Vector3   PreviousCameraFront = {};
		Mumble::Identity  PreviousIdentity = Mumble::Identity{};
		long long         PreviousFrameCounter = 0;

		///----------------------------------------------------------------------------------------------------
		/// AdvanceIdentity:
		/// 	Thread function to parse the mumble identity.
		///----------------------------------------------------------------------------------------------------
		void AdvanceIdentity();

		///----------------------------------------------------------------------------------------------------
		/// AdvanceDerived:
		/// 	Thread function to update derived states.
		///----------------------------------------------------------------------------------------------------
		void AdvanceDerived();
	};
}

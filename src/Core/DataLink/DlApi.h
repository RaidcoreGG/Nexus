///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DlApi.h
/// Description  :  Provides functions to share data and functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "DlLinkedResource.h"
#include "Core/Logging/LogApi.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::Core Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::Core
{
	///----------------------------------------------------------------------------------------------------
	/// DataLinkApi Class
	///----------------------------------------------------------------------------------------------------
	class DataLinkApi
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		DataLinkApi(LogApi& aLogger);
		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~DataLinkApi();

		///----------------------------------------------------------------------------------------------------
		/// Get:
		/// 	Retrieves the resource with the given identifier.
		///----------------------------------------------------------------------------------------------------
		void* Get(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// Share:
		/// 	Allocates memory of the given size, accessible via the provided identifier,
		/// 	but with a different internal/underlying name.
		///----------------------------------------------------------------------------------------------------
		void* Share(
			const char* aIdentifier,
			size_t      aResourceSize,
			const char* aUnderlyingName = "",
			bool        aIsPublic = false
		);

		///----------------------------------------------------------------------------------------------------
		/// GetRegistry:
		/// 	Returns a copy of the registry.
		///----------------------------------------------------------------------------------------------------
		std::unordered_map<std::string, LinkedResource_t> GetRegistry() const;

		private:
		LogApi& Logger;

		mutable std::mutex                                Mutex;
		std::unordered_map<std::string, LinkedResource_t> Registry;
	};
}

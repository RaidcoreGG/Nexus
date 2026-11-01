///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExePackFile.h
/// Description  :  Read-only access to files embedded as PE resources.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <span>

///----------------------------------------------------------------------------------------------------
/// Raidcore::Core::Platform::Windows Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Core::Platform::Windows
{
	///----------------------------------------------------------------------------------------------------
	/// EPackFileType Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class EPackFileType : uint32_t
	{
		Binary,
		Text,
		Font,
		Audio,
		Image,
		Video
	};

	///----------------------------------------------------------------------------------------------------
	/// CPackFile Class
	/// 	A file embedded as a PE resource. Usable at any time, no initialization required.
	///----------------------------------------------------------------------------------------------------
	class CPackFile
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		/// 	Named entry. A null module refers to the module this code is linked into.
		///----------------------------------------------------------------------------------------------------
		constexpr CPackFile(const char* aName, EPackFileType aType, HMODULE aModule = nullptr)
		{
			this->Name   = aName;
			this->Type   = aType;
			this->Module = aModule;
		}

		///----------------------------------------------------------------------------------------------------
		/// ctor
		/// 	Entry by numeric ID and raw resource type, e.g. addons passing their module and resource ID
		/// 	through the API. Their types (e.g. "PNG", RT_FONT) are defined by the addon, not by EPackFileType.
		///----------------------------------------------------------------------------------------------------
		CPackFile(uint32_t aID, LPCSTR aResourceType, HMODULE aModule);

		///----------------------------------------------------------------------------------------------------
		/// GetBytes:
		/// 	Returns the contents of the entry, or an empty span if it does not exist.
		/// 	The memory is owned by the module and stays valid as long as it is loaded.
		///----------------------------------------------------------------------------------------------------
		std::span<const uint8_t> GetBytes() const;

		///----------------------------------------------------------------------------------------------------
		/// Unpack:
		/// 	Writes the entry to disk, replacing an existing file. Returns true on success.
		///----------------------------------------------------------------------------------------------------
		bool Unpack(const std::filesystem::path& aPath) const;

		private:
		LPCSTR        Name         = nullptr; /* String name, or an ID via MAKEINTRESOURCE. */
		EPackFileType Type         = EPackFileType::Binary;
		LPCSTR        ResourceType = nullptr; /* Overrides Type, set for entries by ID. */
		HMODULE       Module       = nullptr;
	};
}

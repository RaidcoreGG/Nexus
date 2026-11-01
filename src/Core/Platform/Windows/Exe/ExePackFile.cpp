///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExePackFile.cpp
/// Description  :  Read-only access to files embedded as PE resources.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExePackFile.h"

#include <fstream>

/* Linker-provided base of the module this code is linked into, so no handle has to be stored. */
extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace Raidcore::Core::Platform::Windows
{
	///----------------------------------------------------------------------------------------------------
	/// GetResourceType:
	/// 	Maps an entry type to the resource type Nexus.rc emits for it (Pack##type, see Resources.inc).
	///----------------------------------------------------------------------------------------------------
	static LPCSTR GetResourceType(EPackFileType aType)
	{
		switch (aType)
		{
			case EPackFileType::Binary: { return "PackBinary"; }
			case EPackFileType::Text:   { return "PackText"; }
			case EPackFileType::Font:   { return "PackFont"; }
			case EPackFileType::Audio:  { return "PackAudio"; }
			case EPackFileType::Image:  { return "PackImage"; }
			case EPackFileType::Video:  { return "PackVideo"; }
		}

		return nullptr;
	}

	CPackFile::CPackFile(uint32_t aID, LPCSTR aResourceType, HMODULE aModule)
	{
		this->Name         = MAKEINTRESOURCEA(aID);
		this->ResourceType = aResourceType;
		this->Module       = aModule;
	}

	std::span<const uint8_t> CPackFile::GetBytes() const
	{
		HMODULE module = this->Module ? this->Module : reinterpret_cast<HMODULE>(&__ImageBase);

		LPCSTR type = this->ResourceType ? this->ResourceType : GetResourceType(this->Type);
		if (!type)
		{
			return {};
		}

		HRSRC hRes = FindResourceA(module, this->Name, type);
		if (!hRes)
		{
			return {};
		}

		HGLOBAL hLRes = LoadResource(module, hRes);
		if (!hLRes)
		{
			return {};
		}

		LPVOID pLRes = LockResource(hLRes);
		if (!pLRes)
		{
			return {};
		}

		DWORD dwResSz = SizeofResource(module, hRes);
		if (!dwResSz)
		{
			return {};
		}

		return { static_cast<const uint8_t*>(pLRes), dwResSz };
	}

	bool CPackFile::Unpack(const std::filesystem::path& aPath) const
	{
		std::span<const uint8_t> bytes = this->GetBytes();

		if (bytes.empty())
		{
			return false;
		}

		try
		{
			/* Binary so the entry is written byte for byte, text mode would expand line endings. */
			std::ofstream file(aPath, std::ios::binary | std::ios::trunc);
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

			return file.good();
		}
		catch (...)
		{
			return false;
		}
	}
}

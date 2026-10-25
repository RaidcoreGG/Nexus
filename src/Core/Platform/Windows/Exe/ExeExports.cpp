///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeExports.cpp
/// Description  :  Reads the exports of a PE file without loading it.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExeExports.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>
#include <windows.h>

namespace Raidcore::Core::Platform::Windows
{
	///----------------------------------------------------------------------------------------------------
	/// LoadFile:
	/// 	Reads the whole file into memory, so it is not locked while parsing.
	///----------------------------------------------------------------------------------------------------
	static bool LoadFile(const std::filesystem::path& aPath, std::vector<uint8_t>& aBuffer)
	{
		std::ifstream file(aPath, std::ios::binary | std::ios::ate);

		if (!file.is_open()) { return false; }

		std::streamoff size = file.tellg();

		if (size <= 0) { return false; }

		aBuffer.resize(static_cast<size_t>(size));
		file.seekg(0, std::ios::beg);

		return static_cast<bool>(file.read(reinterpret_cast<char*>(aBuffer.data()), size));
	}

	///----------------------------------------------------------------------------------------------------
	/// GetData:
	/// 	Returns a pointer to aSize bytes at aOffset, or nullptr if they are not within the buffer.
	///----------------------------------------------------------------------------------------------------
	template <typename T>
	static const T* GetData(const std::vector<uint8_t>& aBuffer, size_t aOffset, size_t aSize)
	{
		if (aOffset > aBuffer.size())         { return nullptr; }
		if (aSize > aBuffer.size() - aOffset) { return nullptr; }

		return reinterpret_cast<const T*>(aBuffer.data() + aOffset);
	}

	///----------------------------------------------------------------------------------------------------
	/// RvaToOffset:
	/// 	Translates a relative virtual address to a file offset. Returns 0 if no section holds it.
	///----------------------------------------------------------------------------------------------------
	static size_t RvaToOffset(const IMAGE_SECTION_HEADER* aSections, WORD aSectionCount, DWORD aRVA)
	{
		for (WORD i = 0; i < aSectionCount; i++)
		{
			const IMAGE_SECTION_HEADER& section = aSections[i];

			if (aRVA < section.VirtualAddress)                          { continue; }
			if (aRVA - section.VirtualAddress >= section.SizeOfRawData) { continue; }

			return section.PointerToRawData + (aRVA - section.VirtualAddress);
		}

		return 0;
	}

	///----------------------------------------------------------------------------------------------------
	/// ToArchitecture:
	/// 	Maps the machine type of the file header.
	///----------------------------------------------------------------------------------------------------
	static EArchitecture ToArchitecture(WORD aMachine)
	{
		switch (aMachine)
		{
			case IMAGE_FILE_MACHINE_I386:  { return EArchitecture::x86;     }
			case IMAGE_FILE_MACHINE_AMD64: { return EArchitecture::x64;     }
			case IMAGE_FILE_MACHINE_ARM64: { return EArchitecture::ARM64;   }
			default:                       { return EArchitecture::Unknown; }
		}
	}

	Exports_t ReadExports(const std::filesystem::path& aPath)
	{
		Exports_t result{};

		std::vector<uint8_t> buffer;

		if (!LoadFile(aPath, buffer)) { return result; }

		/* DOS header. */
		const IMAGE_DOS_HEADER* dosHeader = GetData<IMAGE_DOS_HEADER>(buffer, 0, sizeof(IMAGE_DOS_HEADER));

		if (!dosHeader)                                { return result; }
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return result; }
		if (dosHeader->e_lfanew < 0)                   { return result; }

		/* NT headers: signature, file header, optional header. */
		size_t offset = static_cast<size_t>(dosHeader->e_lfanew);

		const DWORD* signature = GetData<DWORD>(buffer, offset, sizeof(DWORD));

		if (!signature || *signature != IMAGE_NT_SIGNATURE) { return result; }

		offset += sizeof(DWORD);

		const IMAGE_FILE_HEADER* fileHeader = GetData<IMAGE_FILE_HEADER>(buffer, offset, sizeof(IMAGE_FILE_HEADER));

		if (!fileHeader) { return result; }

		bool isDll = (fileHeader->Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL;
		bool isExe = (fileHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == IMAGE_FILE_EXECUTABLE_IMAGE;

		if (!(isDll || isExe)) { return result; }

		offset += sizeof(IMAGE_FILE_HEADER);

		/* The optional header is PE32 for 32-bit and PE32+ for 64-bit files, the data directories sit at different offsets. */
		const WORD* magic = GetData<WORD>(buffer, offset, sizeof(WORD));

		if (!magic) { return result; }

		DWORD                       directoryCount = 0;
		const IMAGE_DATA_DIRECTORY* directories    = nullptr;

		if (*magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			const IMAGE_OPTIONAL_HEADER64* optHeader = GetData<IMAGE_OPTIONAL_HEADER64>(buffer, offset, sizeof(IMAGE_OPTIONAL_HEADER64));

			if (!optHeader)                                                         { return result; }
			if (fileHeader->SizeOfOptionalHeader < sizeof(IMAGE_OPTIONAL_HEADER64)) { return result; }

			directoryCount = optHeader->NumberOfRvaAndSizes;
			directories    = optHeader->DataDirectory;
		}
		else if (*magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			const IMAGE_OPTIONAL_HEADER32* optHeader = GetData<IMAGE_OPTIONAL_HEADER32>(buffer, offset, sizeof(IMAGE_OPTIONAL_HEADER32));

			if (!optHeader)                                                         { return result; }
			if (fileHeader->SizeOfOptionalHeader < sizeof(IMAGE_OPTIONAL_HEADER32)) { return result; }

			directoryCount = optHeader->NumberOfRvaAndSizes;
			directories    = optHeader->DataDirectory;
		}
		else
		{
			return result;
		}

		offset += fileHeader->SizeOfOptionalHeader;

		/* Section table, needed to translate addresses to file offsets. */
		WORD sectionCount = fileHeader->NumberOfSections;

		const IMAGE_SECTION_HEADER* sections = GetData<IMAGE_SECTION_HEADER>(buffer, offset, sectionCount * sizeof(IMAGE_SECTION_HEADER));

		if (!sections) { return result; }

		/* From here on the file is a valid Exe/DLL, missing exports just mean empty lists. */
		result.Architecture = ToArchitecture(fileHeader->Machine);

		if (directoryCount <= IMAGE_DIRECTORY_ENTRY_EXPORT) { return result; }

		const IMAGE_DATA_DIRECTORY& exportEntry = directories[IMAGE_DIRECTORY_ENTRY_EXPORT];

		if (exportEntry.VirtualAddress == 0 || exportEntry.Size == 0) { return result; }

		/* Export directory. */
		size_t exportOffset = RvaToOffset(sections, sectionCount, exportEntry.VirtualAddress);

		const IMAGE_EXPORT_DIRECTORY* exportDir = GetData<IMAGE_EXPORT_DIRECTORY>(buffer, exportOffset, sizeof(IMAGE_EXPORT_DIRECTORY));

		if (exportOffset == 0 || !exportDir) { return result; }

		/* Bound the counts first, so the table sizes below cannot overflow on 32-bit. */
		if (exportDir->NumberOfFunctions > buffer.size() / sizeof(DWORD)) { return result; }
		if (exportDir->NumberOfNames     > buffer.size() / sizeof(DWORD)) { return result; }

		/* Function table: one address per ordinal (Base + index), 0 for unused slots. */
		size_t functionsOffset = RvaToOffset(sections, sectionCount, exportDir->AddressOfFunctions);

		const DWORD* functions = GetData<DWORD>(buffer, functionsOffset, exportDir->NumberOfFunctions * sizeof(DWORD));

		if (functionsOffset == 0 || !functions) { return result; }

		/* Name table and name ordinal table: parallel arrays, name i belongs to function index nameIndices[i]. */
		const DWORD* nameAddresses = nullptr;
		const WORD*  nameIndices   = nullptr;

		if (exportDir->NumberOfNames > 0)
		{
			size_t namesOffset   = RvaToOffset(sections, sectionCount, exportDir->AddressOfNames);
			size_t indicesOffset = RvaToOffset(sections, sectionCount, exportDir->AddressOfNameOrdinals);

			nameAddresses = GetData<DWORD>(buffer, namesOffset,   exportDir->NumberOfNames * sizeof(DWORD));
			nameIndices   = GetData<WORD>(buffer, indicesOffset, exportDir->NumberOfNames * sizeof(WORD));

			if (namesOffset == 0 || !nameAddresses) { return result; }
			if (indicesOffset == 0 || !nameIndices) { return result; }
		}

		std::vector<bool> isNamed(exportDir->NumberOfFunctions, false);

		for (DWORD i = 0; i < exportDir->NumberOfNames; i++)
		{
			WORD functionIndex = nameIndices[i];

			if (functionIndex >= exportDir->NumberOfFunctions) { continue; }

			DWORD ordinal = exportDir->Base + functionIndex;

			if (ordinal > UINT16_MAX) { continue; }

			size_t nameOffset = RvaToOffset(sections, sectionCount, nameAddresses[i]);

			if (nameOffset == 0 || nameOffset >= buffer.size()) { continue; }

			const char* name   = (const char*)buffer.data() + nameOffset;
			size_t      maxLen = buffer.size() - nameOffset;
			size_t      len    = strnlen(name, maxLen);

			/* Not terminated within the file. */
			if (len == maxLen) { continue; }

			result.Names.emplace(std::string(name, len), static_cast<uint16_t>(ordinal));
			isNamed[functionIndex] = true;
		}

		for (DWORD i = 0; i < exportDir->NumberOfFunctions; i++)
		{
			if (functions[i] == 0) { continue; }
			if (isNamed[i])        { continue; }

			DWORD ordinal = exportDir->Base + i;

			if (ordinal > UINT16_MAX) { continue; }

			result.UnnamedOrdinals.push_back(static_cast<uint16_t>(ordinal));
		}

		return result;
	}
}

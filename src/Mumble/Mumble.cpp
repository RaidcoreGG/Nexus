#include "Mumble.h"

namespace Mumble
{
	static HANDLE		Handle;
	static LinkedMem*	Data;

	LinkedMem* Create()
	{
		if (Handle && Data) { return Data; }

		std::wstring mumble_name = GetMumbleName();

		Handle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, mumble_name.c_str());
		if (Handle == 0)
		{
			Handle = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(LinkedMem), mumble_name.c_str());
		}

		if (Handle)
		{
			Data = (LinkedMem*)MapViewOfFile(Handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
			return Data;
		}

		return nullptr;
	}

	void Destroy()
	{
		if (Data)
		{
			UnmapViewOfFile((LPVOID)Data);
			Data = nullptr;
		}

		if (Handle)
		{
			CloseHandle(Handle);
			Handle = nullptr;
		}
	}

	std::wstring GetMumbleName()
	{
		static std::wstring const command = L"-mumble";
		std::wstring commandLine = GetCommandLineW();

		size_t index = commandLine.find(command, 0);

		if (index != std::wstring::npos)
		{
			if (index + command.length() < commandLine.length())
			{
				auto const start = index + command.length() + 1;
				auto const end = commandLine.find(' ', start);
				std::wstring mumble = commandLine.substr(start, (end != std::wstring::npos ? end : commandLine.length()) - start);

				return mumble;
			}
		}

		return L"MumbleLink";
	}
}
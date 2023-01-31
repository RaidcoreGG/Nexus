#include "Keybind.h"

std::wstring Keybind::ToString()
{
	if (!Key) { return L"(null)"; }

	std::wstring str;

	str.append(Alt ? L"ALT+" : L"");
	str.append(Ctrl ? L"CTRL+" : L"");
	str.append(Shift ? L"SHIFT+" : L"");

	wchar_t* buff = new wchar_t[10];
	GetKeyNameTextW(MapVirtualKeyW(Key, MAPVK_VK_TO_VSC) << 16, buff, 10);
	str.append(buff);

	return str;
}

bool operator==(const Keybind& lhs, const Keybind& rhs)
{
	return	lhs.Key		== rhs.Key &&
			lhs.Alt		== rhs.Alt &&
			lhs.Ctrl	== rhs.Ctrl &&
			lhs.Shift	== rhs.Shift;
}

bool operator!=(const Keybind& lhs, const Keybind& rhs)
{
	return	!(lhs == rhs);
}

Keybind KBFromString(std::wstring aKeybind)
{
	Keybind kb{};

	if (wcscmp(aKeybind.c_str(), L"(null)") == 0 || wcscmp(aKeybind.c_str(), L"(NULL)") == 0) { return kb; }

	std::transform(aKeybind.begin(), aKeybind.end(), aKeybind.begin(), ::toupper);
	std::wstring delimiter = L"+";

	size_t pos = 0;
	std::wstring token;
	while ((pos = aKeybind.find(delimiter)) != std::string::npos)
	{
		token = aKeybind.substr(0, pos);
		aKeybind.erase(0, pos + delimiter.length());
		
		if (wcscmp(token.c_str(), L"ALT") == 0)
		{
			kb.Alt = true;
		}
		else if (wcscmp(token.c_str(), L"CTRL") == 0)
		{
			kb.Ctrl = true;
		}
		else if (wcscmp(token.c_str(), L"SHIFT") == 0)
		{
			kb.Shift = true;
		}
	}

	kb.Key = (char)VkKeyScanW(aKeybind.c_str()[0]);

	return kb;
}
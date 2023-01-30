#include "Keybind.h"

std::wstring Keybind::ToString()
{
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

Keybind KBFromString(const wchar_t* aKeybind)
{
	Keybind kb{};

	std::wstring str = aKeybind;
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	std::wstring delimiter = L"+";

	size_t pos = 0;
	std::wstring token;
	while ((pos = str.find(delimiter)) != std::wstring::npos)
	{
		token = str.substr(0, pos);
		str.erase(0, pos + delimiter.length());
		
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

	kb.Key = (char)VkKeyScanW(str.c_str()[0]);

	return kb;
}
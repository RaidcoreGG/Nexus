#include "Keybind.h"

std::string Keybind::ToString(bool padded)
{
	if (!Key) { return "(null)"; }

	char* buff = new char[16];
	std::string str;

	if (Alt)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_MENU, MAPVK_VK_TO_VSC) << 16, buff, 16);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	if (Ctrl)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC) << 16, buff, 16);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	if (Shift)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC) << 16, buff, 16);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	GetKeyNameTextA(MapVirtualKeyA(Key, MAPVK_VK_TO_VSC) << 16, buff, 16);
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

std::map<std::string, char> LookupTable
{
	{"BACKSPACE", 8},
	{"TAB", 9},
	{"ENTER", 13},
	{"CAPS LOCK", 20},
	{"ESC", 27},
	{"END", 35},
	{"HOME", 36},
	{"INSERT", 45},
	{"DELETE", 46},
	{"F1", 112},
	{"F2", 113},
	{"F3", 114},
	{"F4", 115},
	{"F5", 116},
	{"F6", 117},
	{"F7", 118},
	{"F8", 119},
	{"F9", 120},
	{"F10", 121},
	{"F11", 122},
	{"F12", 123},
};

Keybind KBFromString(std::string aKeybind)
{
	Keybind kb{};

	if (aKeybind == "(null)" || aKeybind == "(NULL)") { return kb; }

	std::transform(aKeybind.begin(), aKeybind.end(), aKeybind.begin(), ::toupper);
	std::string delimiter = "+";

	size_t pos = 0;
	std::string token;
	while ((pos = aKeybind.find(delimiter)) != std::string::npos)
	{
		token = aKeybind.substr(0, pos);
		aKeybind.erase(0, pos + delimiter.length());
		
		if (token == "ALT")
		{
			kb.Alt = true;
		}
		else if (token == "CTRL")
		{
			kb.Ctrl = true;
		}
		else if (token == "SHIFT")
		{
			kb.Shift = true;
		}
	}

	if (LookupTable.find(aKeybind) != LookupTable.end())
	{
		kb.Key = LookupTable[aKeybind];
	}
	else
	{
		kb.Key = (char)VkKeyScanA(aKeybind.c_str()[0]);
	}

	return kb;
}
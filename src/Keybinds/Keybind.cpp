#include "Keybind.h"

std::string Keybind::ToString(bool padded)
{
	if (!Key) { return "(null)"; }

	std::string str;

	if (padded)
	{

		str.append(Alt ? "ALT + " : "");
		str.append(Ctrl ? "CTRL + " : "");
		str.append(Shift ? "SHIFT + " : "");
	}
	else
	{
		str.append(Alt ? "ALT+" : "");
		str.append(Ctrl ? "CTRL+" : "");
		str.append(Shift ? "SHIFT+" : "");
	}

	char* buff = new char[10];
	GetKeyNameTextA(MapVirtualKeyA(Key, MAPVK_VK_TO_VSC) << 16, buff, 10);
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

	kb.Key = (char)VkKeyScanA(aKeybind.c_str()[0]);

	return kb;
}
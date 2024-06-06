#include "Keybind.h"

#include <Windows.h>
#include <algorithm>
#include <map>

#include "core.h"

#include "KeybindHandler.h"

std::string Keybind::ToString(bool padded)
{
	if (!Key) { return "(null)"; }

	char* buff = new char[100];
	std::string str;

	if (Alt)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_MENU, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	if (Ctrl)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	if (Shift)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(padded ? " + " : "+");
	}

	HKL hkl = GetKeyboardLayout(0);
	UINT vk = MapVirtualKeyA(Key, MAPVK_VSC_TO_VK);

	if (vk >= 65 && vk <= 90 || vk >= 48 && vk <= 57)
	{
		GetKeyNameTextA(Key << 16, buff, 100);
		str.append(buff);
	}
	else
	{
		auto it = Keybinds::ScancodeLookupTable.find(Key);
		if (it != Keybinds::ScancodeLookupTable.end())
		{
			str.append(it->second);
		}
	}

	delete[] buff;

	str = String::ToUpper(str);

	// Convert Multibyte encoding to UFT-8 bytes
	const char* multibyte_pointer = str.c_str();
	const char* utf8_bytes = ConvertToUTF8(multibyte_pointer);
	
	return std::string(utf8_bytes);
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
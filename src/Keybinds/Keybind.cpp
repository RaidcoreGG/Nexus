#include "Keybind.h"

#include "KeybindHandler.h"
#include "../core.h"

const char* ConvertToUTF8(const char* multibyteStr)
{
	char* utf8Str = nullptr;

	int wideCharCount = MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, NULL, 0);
	if (wideCharCount > 0)
	{
		wchar_t* wideCharBuff = new wchar_t[wideCharCount];
		MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, wideCharBuff, wideCharCount);

		int utf8Count = WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, NULL, 0, NULL, NULL);
		if (utf8Count > 0)
		{
			utf8Str = new char[utf8Count];
			WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, utf8Str, utf8Count, NULL, NULL);
		}

		delete[] wideCharBuff;
	}

	return utf8Str;
}


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
		//UINT keyExW = MapVirtualKeyExW(Key, MAPVK_VSC_TO_VK_EX, hkl);
		//str.append(std::to_string(keyExW));
		GetKeyNameTextA(Key << 16, buff, 100);
		str.append(buff);
	}
	else
	{
		// TODO:
		// this does not work for special utf8 characters
		// basically ImGui::Text(u8"somestring") -> handles utf8 properly
		// since this is a std::string, there is no prefixing, meaning it has to be converted somehow, no idea how
		// characters like Ü or ß don't work. I give up
		auto it = Keybinds::ScancodeLookupTable.find(Key);
		if (it != Keybinds::ScancodeLookupTable.end())
		{
			str.append(it->second);
		}

		/*// get vkey from keycode based on the current keyboardlayout
		UINT keyExW = MapVirtualKeyExW(Key, MAPVK_VSC_TO_VK_EX, hkl);
		wchar_t shortCutRealNameWstr[32];
		constexpr BYTE keyState[256]{};
		// say windows to get the translation of the key (e.g. ä, ö, #)
		int toUnicodeCount = ToUnicodeEx(keyExW, Key, keyState, shortCutRealNameWstr, 32, 1 << 2, hkl);
		// some keys set two utf16 chars, i only care about the first one, so i set the second one to 0
		if (toUnicodeCount == 2) {
			shortCutRealNameWstr[2] = '\0';
		}

		int count = WideCharToMultiByte(CP_UTF8, 0, shortCutRealNameWstr, sizeof(shortCutRealNameWstr), NULL, 0, NULL, NULL);
		std::string str2(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, shortCutRealNameWstr, -1, &str2[0], count, NULL, NULL);
		std::transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
		str.append(std::to_string(toUnicodeCount));
		if (toUnicodeCount == 0)
		{
			str.append("nt");
		}
		else if (toUnicodeCount < 0)
		{
			str.append("dk");
		}
		str.append(str2);*/
	}

	delete[] buff;

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
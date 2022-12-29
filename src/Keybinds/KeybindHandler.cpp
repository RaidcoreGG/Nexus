#include <cstdarg>

#include "KeybindHandler.h"

#include "../core.h"
#include "../Paths.h"
#include "../Shared.h"

using json = nlohmann::json;

bool operator==(const Keybind& lhs, const Keybind& rhs)
{
	return	lhs.Key		== rhs.Key &&
			lhs.Alt		== rhs.Alt &&
			lhs.Ctrl	== rhs.Ctrl &&
			lhs.Shift	== rhs.Shift;
}

namespace KeybindHandler
{
	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Keybind kb{};
		kb.Alt		= GetKeyState(VK_MENU)		& 0x8000;
		kb.Ctrl		= GetKeyState(VK_CONTROL)	& 0x8000;
		kb.Shift	= GetKeyState(VK_SHIFT)		& 0x8000;

		switch (uMsg)
		{
		case WM_SYSKEYDOWN: case WM_KEYDOWN:
			if (wParam > 255) break;
			kb.Key = wParam;
			
			for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
			{
				Keybind stored = it->second;

				if (kb == stored)
				{
					InvokeKeybind(it->first);
					return true;
				}
			}
		}

		return false;
	}

	void LoadKeybinds()
	{
		KeybindRegistryMutex.lock();

		std::ifstream file(Path::F_KEYBINDS_JSON);

		json keybinds = json::parse(file);

		File.close();

		for (json binding : keybinds)
		{
			Keybind kb{};
			kb.Key = binding["Key"].get<WORD>();
			kb.Alt = binding["Alt"].get<bool>();
			kb.Ctrl = binding["Ctrl"].get<bool>();
			kb.Shift = binding["Shift"].get<bool>();

			char str[MAX_PATH]{};
			memcpy(str, binding["Identifier"].get<std::string>().c_str(), MAX_PATH);

			size_t sz = strlen(str) + 1;
			wchar_t* wc = new wchar_t[sz];
			mbstowcs_s(nullptr, wc, sz, str, sz);

			KeybindRegistry[wc] = kb;
		}

		KeybindRegistryMutex.unlock();
	}

	void SaveKeybinds()
	{
		KeybindRegistryMutex.lock();

		json keybinds = json::array();

		for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
		{
			Keybind kb = it->second;
			std::wstring id = it->first;

			json binding =
			{
				{"Identifier",	WStrToStr(id)},
				{"Key",			kb.Key},
				{"Alt",			kb.Alt},
				{"Ctrl",		kb.Ctrl},
				{"Shift",		kb.Shift}
			};

			keybinds.push_back(binding);
		}

		File.open(Path::F_KEYBINDS_JSON);

		// copy fuckery because nlohmann::json only supports utf8 obviously
		char str[MAX_PATH]{};
		memcpy(str, keybinds.dump(4).c_str(), MAX_PATH);
		size_t sz = strlen(str) + 1;
		wchar_t* wc = new wchar_t[sz];
		mbstowcs_s(nullptr, wc, sz, str, sz);

		File << wc << std::endl;
		File.close();

		KeybindRegistryMutex.unlock();
	}

	void RegisterKeybind(const wchar_t* aIdentifier, KeybindHandlerSig aKeybindHandler, Keybind aKeybind)
	{
		KeybindRegistryMutex.lock();

		/* set keybind handler for given identifier */
		KeybindHandlerRegistry[aIdentifier] = aKeybindHandler;

		/* check if keybind already registered (disk/previous init) ? ignore, use saved one : attempt to register */
		if (KeybindRegistry[aIdentifier].Key == 0)
		{
			/* check if another identifier, already uses the keybind */
			for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
			{
				if (it->second == aKeybind)
				{
					/* Keybind already exists, don't overwrite, initialise empty keybind instead, so it can be set from the menu */
					KeybindRegistry[aIdentifier] = {};
					KeybindRegistryMutex.unlock();
					return;
				}
			}

			KeybindRegistry[aIdentifier] = aKeybind;
		}

		KeybindRegistryMutex.unlock();
	}

	void InvokeKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		if (KeybindHandlerRegistry[aIdentifier])
		{
			KeybindHandlerRegistry[aIdentifier](aIdentifier);
		}
		else
		{
			Logger->LogInfo(L"Stale keybind: %s", aIdentifier);
		}

		KeybindRegistryMutex.unlock();
	}
}
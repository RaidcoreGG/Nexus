#include <cstdarg>

#include "KeybindHandler.h"

#include "../core.h"
#include "../Paths.h"
#include "../Shared.h"
#include "../State.h"

using json = nlohmann::json;

namespace KeybindHandler
{
	std::mutex KeybindRegistryMutex;
	std::map<const wchar_t*, Keybind> KeybindRegistry;
	std::map<const wchar_t*, KEYBINDS_PROCESS> KeybindHandlerRegistry;

	std::mutex HeldKeysMutex;
	std::vector<WPARAM> HeldKeys;

	std::wfstream File;

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
			
			HeldKeysMutex.lock();

			if (std::find(HeldKeys.begin(), HeldKeys.end(), wParam) != HeldKeys.end())
			{
				HeldKeysMutex.unlock();
				return false;
			}
			HeldKeys.push_back(wParam);

			HeldKeysMutex.unlock();
			
			for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
			{
				Keybind stored = it->second;

				if (kb == stored)
				{
					InvokeKeybind(it->first);
					return true;
				}
			}
		case WM_SYSKEYUP: case WM_KEYUP:
			HeldKeysMutex.lock();

			HeldKeys.erase(std::remove(HeldKeys.begin(), HeldKeys.end(), wParam), HeldKeys.end());

			HeldKeysMutex.unlock();
			break;
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

		std::thread([]()
			{
				int i = 0;
				while (State::AddonHost < ggState::ADDONS_READY)
				{
					// check every 100ms, abort after 1 minute
					if (i == 600)
					{
						LogWarning(L"Addons not initialized after 60 seconds cancelling stale keybind validation.");
						return;
					}
					Sleep(100);
					i++;
				}

				ValidateKeybinds();

			}).detach();
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
		memcpy(str, keybinds.dump(1, '\t').c_str(), MAX_PATH);
		size_t sz = strlen(str) + 1;
		wchar_t* wc = new wchar_t[sz];
		mbstowcs_s(nullptr, wc, sz, str, sz);

		File << wc << std::endl;
		File.close();

		KeybindRegistryMutex.unlock();
	}

	void ValidateKeybinds()
	{
		KeybindRegistryMutex.lock();
		for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
		{
			if (!KeybindHandlerRegistry[it->first])
			{
				LogInfo(L"Stale keybind: %s", it->first);
			}
		}
		KeybindRegistryMutex.unlock();
	}

	void RegisterKeybind(const wchar_t* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, Keybind aKeybind)
	{
		KeybindRegistryMutex.lock();

		/* set keybind handler for given identifier */
		KeybindHandlerRegistry[aIdentifier] = aKeybindHandler;

		/* check if keybind already registered (disk/previous init) ? ignore, use saved one : attempt to register */
		if (KeybindRegistry.find(aIdentifier) != KeybindRegistry.end() && KeybindRegistry[aIdentifier].Key == 0)
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
		}

		KeybindRegistry[aIdentifier] = aKeybind;

		KeybindRegistryMutex.unlock();
	}

	void UnregisterKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		KeybindRegistry.erase(aIdentifier);
		KeybindHandlerRegistry.erase(aIdentifier);

		KeybindRegistryMutex.unlock();
	}

	void InvokeKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		if (KeybindHandlerRegistry[aIdentifier])
		{
			KeybindHandlerRegistry[aIdentifier](aIdentifier);
		}

		KeybindRegistryMutex.unlock();
	}
}
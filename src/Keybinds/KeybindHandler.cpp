#include <cstdarg>

#include "KeybindHandler.h"

#include "../core.h"
#include "../Paths.h"
#include "../Shared.h"
#include "../State.h"

namespace KeybindHandler
{
	std::mutex KeybindRegistryMutex;
	std::map<std::string, Keybind> KeybindRegistry;
	std::map<std::string, KEYBINDS_PROCESS> KeybindHandlerRegistry;

	std::mutex HeldKeysMutex;
	std::vector<WPARAM> HeldKeys;

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
			
			for (std::map<std::string, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
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
		if (!std::filesystem::exists(Path::F_KEYBINDS)) { return; }

		KeybindRegistryMutex.lock();

		std::ifstream file(Path::F_KEYBINDS);

		std::string line;
		while (std::getline(file, line))
		{
			std::transform(line.begin(), line.end(), line.begin(), ::toupper);
			std::string delimiter = "=";

			size_t pos = 0;
			std::string token;
			while ((pos = line.find(delimiter)) != std::string::npos)
			{
				token = line.substr(0, pos);
				line.erase(0, pos + delimiter.length());
			}

			KeybindRegistry[token] = KBFromString(line);
			LogDebug("%s | %s", token.c_str(), line.c_str());
		}

		file.close();

		KeybindRegistryMutex.unlock();
	}

	void SaveKeybinds()
	{
		KeybindRegistryMutex.lock();

		std::ofstream file(Path::F_KEYBINDS);

		for (std::map<std::string, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
		{
			std::string id = it->first;
			std::string kb = it->second.ToString();

			file.write(id.c_str(), id.size());
			file.write("=", 1);
			file.write(kb.c_str(), kb.size());
			file.write("\n", 1);
		}

		file.close();

		KeybindRegistryMutex.unlock();
	}

	void RegisterKeybind(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		KeybindRegistryMutex.lock();

		/* check if another identifier, already uses the keybind */
		for (auto& [identifier, keybind] : KeybindRegistry)
		{
			if (keybind == requestedBind)
			{
				if (identifier != aIdentifier)
				{
					/* another identifier uses the same combination */
					requestedBind = {};
				}

				break;
			}
		}

		/* check if this keybind is not already set */
		if (KeybindRegistry.find(aIdentifier) == KeybindRegistry.end())
		{
			KeybindRegistry[aIdentifier] = requestedBind;
		}
		else
		{
			if (KeybindRegistry[aIdentifier] == Keybind{})
			{
				KeybindRegistry[aIdentifier] = requestedBind;
			}
		}

		KeybindHandlerRegistry[aIdentifier] = aKeybindHandler;

		KeybindRegistryMutex.unlock();

		SaveKeybinds();
	}

	void UnregisterKeybind(std::string aIdentifier)
	{
		KeybindRegistryMutex.lock();

		KeybindRegistry.erase(aIdentifier);
		KeybindHandlerRegistry.erase(aIdentifier);

		KeybindRegistryMutex.unlock();
	}

	void SetKeybind(std::string aIdentifier, std::string aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		KeybindRegistryMutex.lock();

		KeybindRegistry[aIdentifier] = requestedBind;

		KeybindRegistryMutex.unlock();
	}

	void InvokeKeybind(std::string aIdentifier)
	{
		KeybindRegistryMutex.lock();

		if (KeybindHandlerRegistry[aIdentifier])
		{
			KeybindHandlerRegistry[aIdentifier](aIdentifier);
		}

		KeybindRegistryMutex.unlock();
	}
}
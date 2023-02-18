#include "KeybindHandler.h"

namespace Keybinds
{
	std::mutex Mutex;
	std::map<std::string, Keybind> Registry;
	std::map<std::string, KEYBINDS_PROCESS> HandlerRegistry;

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
			
			for (std::map<std::string, Keybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
			{
				Keybind stored = it->second;

				if (kb == stored)
				{
					Invoke(it->first);
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

	void Load()
	{
		if (!std::filesystem::exists(Path::F_KEYBINDS)) { return; }

		Mutex.lock();

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

			Registry[token] = KBFromString(line);
			LogDebug("%s | %s", token.c_str(), line.c_str());
		}

		file.close();

		Mutex.unlock();
	}

	void Save()
	{
		Mutex.lock();

		std::ofstream file(Path::F_KEYBINDS);

		for (std::map<std::string, Keybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
		{
			std::string id = it->first;
			std::string kb = it->second.ToString();

			file.write(id.c_str(), id.size());
			file.write("=", 1);
			file.write(kb.c_str(), kb.size());
			file.write("\n", 1);
		}

		file.close();

		Mutex.unlock();
	}

	void Register(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		Mutex.lock();

		/* check if another identifier, already uses the keybind */
		for (auto& [identifier, keybind] : Registry)
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
		if (Registry.find(aIdentifier) == Registry.end())
		{
			Registry[aIdentifier] = requestedBind;
		}
		else
		{
			if (Registry[aIdentifier] == Keybind{})
			{
				Registry[aIdentifier] = requestedBind;
			}
		}

		HandlerRegistry[aIdentifier] = aKeybindHandler;

		Mutex.unlock();

		Save();
	}

	void Unregister(std::string aIdentifier)
	{
		Mutex.lock();

		Registry.erase(aIdentifier);
		HandlerRegistry.erase(aIdentifier);

		Mutex.unlock();
	}

	void Set(std::string aIdentifier, std::string aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		Mutex.lock();

		Registry[aIdentifier] = requestedBind;

		Mutex.unlock();
	}

	void Invoke(std::string aIdentifier)
	{
		Mutex.lock();

		if (HandlerRegistry[aIdentifier])
		{
			HandlerRegistry[aIdentifier](aIdentifier);
		}

		Mutex.unlock();
	}
}
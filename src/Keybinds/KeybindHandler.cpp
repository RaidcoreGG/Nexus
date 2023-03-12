#include "KeybindHandler.h"

namespace Keybinds
{
	std::mutex								Mutex;
	std::map<std::string, ActiveKeybind>	Registry;

	std::mutex								HeldKeysMutex;
	std::vector<WPARAM>						HeldKeys;

	bool									IsSettingKeybind = false;
	Keybind									CurrentKeybind;
	std::string								CurrentKeybindUsedBy;

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Keybind kb{};
		kb.Alt		= GetKeyState(VK_MENU)		& 0x8000;
		kb.Ctrl		= GetKeyState(VK_CONTROL)	& 0x8000;
		kb.Shift	= GetKeyState(VK_SHIFT)		& 0x8000;

		switch (uMsg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (wParam > 255) break;
			kb.Key = wParam;
			
			// if shift, ctrl or alt set key to 0
			if (wParam == 16 || wParam == 17 || wParam == 18)
			{
				kb.Key = 0;
			}

			HeldKeysMutex.lock();
			if (std::find(HeldKeys.begin(), HeldKeys.end(), wParam) != HeldKeys.end())
			{
				HeldKeysMutex.unlock();
				return false;
			}
			HeldKeys.push_back(wParam);
			HeldKeysMutex.unlock();
			
			/* only check if not currently setting keybind */
			if (!IsSettingKeybind)
			{
				for (std::map<std::string, ActiveKeybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
				{
					if (kb == it->second.Bind)
					{
						return Invoke(it->first);
					}
				}
			}
			else
			{
				CurrentKeybind = kb;
				CurrentKeybindUsedBy = Keybinds::IsInUse(kb);
			}
			
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
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

		json keybinds = json::parse(file);

		for (json binding : keybinds)
		{
			Keybind kb{};
			kb.Key = binding["Key"].get<WPARAM>();
			kb.Alt = binding["Alt"].get<bool>();
			kb.Ctrl = binding["Ctrl"].get<bool>();
			kb.Shift = binding["Shift"].get<bool>();

			std::string identifier = binding["Identifier"].get<std::string>();

			Registry[identifier].Bind = kb;
		}

		/*std::string line;
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

			Registry[token].Bind = KBFromString(line);
			LogDebug("%s | %s", token.c_str(), line.c_str());
		}*/

		file.close();

		Mutex.unlock();
	}

	void Save()
	{
		Mutex.lock();


		json keybinds = json::array();

		for (std::map<std::string, ActiveKeybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
		{
			Keybind kb = it->second.Bind;
			std::string id = it->first;

			json binding =
			{
				{"Identifier",	id},
				{"Key",			kb.Key},
				{"Alt",			kb.Alt},
				{"Ctrl",		kb.Ctrl},
				{"Shift",		kb.Shift}
			};

			keybinds.push_back(binding);
		}

		std::ofstream file(Path::F_KEYBINDS);

		/*for (std::map<std::string, ActiveKeybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
		{
			std::string id = it->first;
			std::string kb = it->second.Bind.ToString();

			file.write(id.c_str(), id.size());
			file.write("=", 1);
			file.write(kb.c_str(), kb.size());
			file.write("\n", 1);
		}*/

		file << keybinds.dump(1, '\t') << std::endl;

		file.close();

		Mutex.unlock();
	}

	void Register(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		/* check if another identifier, already uses the keybind */
		std::string res = IsInUse(requestedBind);

		if (res != aIdentifier && res != "")
		{
			/* another identifier uses the same combination */
			requestedBind = {};
		}

		Mutex.lock();
		/* check if this keybind is not already set */
		if (Registry.find(aIdentifier) == Registry.end())
		{
			Registry[aIdentifier].Bind = requestedBind;
		}

		Registry[aIdentifier].Handler = aKeybindHandler;
		Mutex.unlock();

		Save();
	}

	void Unregister(std::string aIdentifier)
	{
		Mutex.lock();

		Registry.erase(aIdentifier);

		Mutex.unlock();

		Save();
	}

	std::string IsInUse(Keybind aKeybind)
	{
		/* sanity check */
		if (aKeybind == Keybind{}) { return ""; }

		Mutex.lock();
		/* check if another identifier, already uses the keybind */
		for (auto& [identifier, keybind] : Registry)
		{
			if (keybind.Bind == aKeybind)
			{
				Mutex.unlock();
				return identifier;
			}
		}
		Mutex.unlock();

		return "";
	}

	void Set(std::string aIdentifier, Keybind aKeybind)
	{
		std::string res = IsInUse(aKeybind);

		if (res != aIdentifier && res != "") { return; }

		Mutex.lock();
		Registry[aIdentifier].Bind = aKeybind;
		Mutex.unlock();

		Save();
	}

	bool Invoke(std::string aIdentifier)
	{
		bool called = false;

		Mutex.lock();
		if (Registry[aIdentifier].Handler)
		{
			Registry[aIdentifier].Handler(aIdentifier);
			called = true;
		}
		Mutex.unlock();

		return called;
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		for (auto& [identifier, activekb] : Registry)
		{
			if (activekb.Handler >= aStartAddress && activekb.Handler <= aEndAddress)
			{
				activekb.Handler = nullptr;
				refCounter++;
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}
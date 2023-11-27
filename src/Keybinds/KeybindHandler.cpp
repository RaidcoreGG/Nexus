#include "KeybindHandler.h"

namespace Keybinds
{
	std::mutex								Mutex;
	std::map<std::string, ActiveKeybind>	Registry;

	bool									IsSettingKeybind = false;
	Keybind									CurrentKeybind;
	std::string								CurrentKeybindUsedBy;

	std::map<unsigned short, std::string>	ScancodeLookupTable;

	void Initialize()
	{
		for (long long i = 0; i < 255; i++)
		{
			KeyLParam key{};
			key.ScanCode = i;
			char* buff = new char[64];
			std::string str;
			GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
			str.append(buff);

			ScancodeLookupTable[key.GetScanCode()] = str;

			key.ExtendedFlag = 1;
			buff = new char[64];
			str = "";
			GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
			str.append(buff);

			ScancodeLookupTable[key.GetScanCode()] = str;

			delete[] buff;
		}

		Load();
	}

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (wParam > 255) break;

			KeyLParam keylp = LParamToKMF(lParam);

			Keybind kb{};
			kb.Alt = GetKeyState(VK_MENU) & 0x8000;
			kb.Ctrl = GetKeyState(VK_CONTROL) & 0x8000;
			kb.Shift = GetKeyState(VK_SHIFT) & 0x8000;
			kb.Key = keylp.GetScanCode();
			
			// if shift, ctrl or alt set key to 0
			if (wParam == 16 || wParam == 17 || wParam == 18)
			{
				kb.Key = 0;
			}

			if (keylp.PreviousKeyState)
			{
				return uMsg;
			}
			
			/* only check if not currently setting keybind */
			if (!IsSettingKeybind)
			{
				for (std::map<std::string, ActiveKeybind>::iterator it = Registry.begin(); it != Registry.end(); ++it)
				{
					if (kb == it->second.Bind)
					{
						// if Invoke returns true, pass 0 to the wndproc to stop processing
						return Invoke(it->first) ? 0 : 1;
					}
				}
			}
			else
			{
				CurrentKeybind = kb;
				CurrentKeybindUsedBy = Keybinds::IsInUse(kb);
			}
			
			break;
		}

		return uMsg;
	}

	void Load()
	{
		if (!std::filesystem::exists(Path::F_KEYBINDS)) { return; }

		Keybinds::Mutex.lock();
		{
			std::ifstream file(Path::F_KEYBINDS);

			json keybinds = json::parse(file);

			for (json binding : keybinds)
			{
				Keybind kb{};
				kb.Key = binding["Key"].get<unsigned>();
				kb.Alt = binding["Alt"].get<bool>();
				kb.Ctrl = binding["Ctrl"].get<bool>();
				kb.Shift = binding["Shift"].get<bool>();

				std::string identifier = binding["Identifier"].get<std::string>();

				Registry[identifier].Bind = kb;
			}

			file.close();
		}
		Keybinds::Mutex.unlock();
	}

	void Save()
	{
		Keybinds::Mutex.lock();
		{
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

			file << keybinds.dump(1, '\t') << std::endl;

			file.close();
		}
		Keybinds::Mutex.unlock();
	}

	void Register(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const char* aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);
		RegisterWithStruct(aIdentifier, aKeybindHandler, requestedBind);
	}
	void RegisterWithStruct(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, Keybind aKeybind)
	{
		std::string str = aIdentifier;

		Keybind requestedBind = aKeybind;

		/* check if another identifier, already uses the keybind */
		std::string res = IsInUse(requestedBind);

		if (res != str && res != "")
		{
			/* another identifier uses the same combination */
			requestedBind = {};
		}

		Keybinds::Mutex.lock();
		{
			/* check if this keybind is not already set */
			if (Registry.find(str) == Registry.end())
			{
				Registry[str].Bind = requestedBind;
			}

			Registry[str].Handler = aKeybindHandler;
		}
		Keybinds::Mutex.unlock();

		Save();
	}
	void Unregister(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		Keybinds::Mutex.lock();
		{
			Registry.erase(str);
		}
		Keybinds::Mutex.unlock();

		Save();
	}

	std::string IsInUse(Keybind aKeybind)
	{
		/* sanity check */
		if (aKeybind == Keybind{}) { return ""; }

		Keybinds::Mutex.lock();
		{
			/* check if another identifier, already uses the keybind */
			for (auto& [identifier, keybind] : Registry)
			{
				if (keybind.Bind == aKeybind)
				{
					Keybinds::Mutex.unlock();
					return identifier;
				}
			}
		}
		Keybinds::Mutex.unlock();

		return "";
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

		for (auto it = ScancodeLookupTable.begin(); it != ScancodeLookupTable.end(); ++it)
		{
			if (it->second == aKeybind)
			{
				kb.Key = it->first;
				break;
			}
		}

		return kb;
	}

	void Set(std::string aIdentifier, Keybind aKeybind)
	{
		std::string res = IsInUse(aKeybind);

		if (res != aIdentifier && res != "") { return; }

		Keybinds::Mutex.lock();
		{
			Registry[aIdentifier].Bind = aKeybind;
		}
		Keybinds::Mutex.unlock();

		Save();
	}
	bool Invoke(std::string aIdentifier)
	{
		bool called = false;

		Keybinds::Mutex.lock();
		{
			if (Registry[aIdentifier].Handler)
			{
				std::thread([aIdentifier]()
					{
						Registry[aIdentifier].Handler(aIdentifier.c_str());
					}).detach();
				called = true;
			}
		}
		Keybinds::Mutex.unlock();

		return called;
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Keybinds::Mutex.lock();
		{
			for (auto& [identifier, activekb] : Registry)
			{
				if (activekb.Handler >= aStartAddress && activekb.Handler <= aEndAddress)
				{
					activekb.Handler = nullptr;
					refCounter++;
				}
			}
		}
		Keybinds::Mutex.unlock();

		return refCounter;
	}
}
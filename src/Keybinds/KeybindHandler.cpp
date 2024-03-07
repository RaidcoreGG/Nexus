#include "KeybindHandler.h"

#include <fstream>
#include <filesystem>
#include <string>
#include <cstdarg>

#include "core.h"
#include "Consts.h"
#include "Paths.h"
#include "Shared.h"
#include "State.h"

#include "KeystrokeMessageFlags.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

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

			if (!kb.Alt && !kb.Ctrl && !kb.Shift && kb.Key == 0)
			{
				return uMsg;
			}

			if (keylp.PreviousKeyState)
			{
				return uMsg;
			}
			
			/* only check if not currently setting keybind */
			if (!IsSettingKeybind)
			{
				for (auto& it : Registry)
				{
					if (kb == it.second.Bind)
					{
						// if Invoke returns true, pass 0 to the wndproc to stop processing
						return Invoke(it.first) ? 0 : 1;
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

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			try
			{
				std::ifstream file(Path::F_KEYBINDS);

				json keybinds = json::parse(file);
				for (json binding : keybinds)
				{
					if (binding.is_null() ||
						binding["Key"].is_null() ||
						binding["Alt"].is_null() ||
						binding["Ctrl"].is_null() ||
						binding["Shift"].is_null())
					{
						LogDebug(CH_KEYBINDS, "One or more fields of keybind were null.");
						continue;
					}

					Keybind kb{};
					binding["Key"].get_to(kb.Key);
					binding["Alt"].get_to(kb.Alt);
					binding["Ctrl"].get_to(kb.Ctrl);
					binding["Shift"].get_to(kb.Shift);

					std::string identifier = binding["Identifier"].get<std::string>();

					Registry[identifier].Bind = kb;
				}

				file.close();
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_KEYBINDS, "Keybinds.json could not be parsed. Error: %s", ex.what());
			}
		}
	}

	void Save()
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		{
			json keybinds = json::array();

			for (auto& it : Registry)
			{
				Keybind kb = it.second.Bind;
				std::string id = it.first;

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

		{
			const std::lock_guard<std::mutex> lock(Mutex);
			/* check if this keybind is not already set */
			if (Registry.find(str) == Registry.end())
			{
				Registry[str].Bind = requestedBind;
			}

			Registry[str].Handler = aKeybindHandler;
		}

		Save();
	}
	void Deregister(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		{
			const std::lock_guard<std::mutex> lock(Mutex);
			auto it = Registry.find(str);

			if (it != Registry.end())
			{
				it->second.Handler = nullptr;
			}
		}

		Save();
	}

	std::string IsInUse(Keybind aKeybind)
	{
		/* sanity check */
		if (aKeybind == Keybind{}) { return ""; }

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			/* check if another identifier, already uses the keybind */
			for (auto& [identifier, keybind] : Registry)
			{
				if (keybind.Bind == aKeybind)
				{
					return identifier;
				}
			}
		}

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

		{
			const std::lock_guard<std::mutex> lock(Mutex);
			Registry[aIdentifier].Bind = aKeybind;
		}

		Save();
	}
	bool Invoke(std::string aIdentifier)
	{
		bool called = false;

		{
			const std::lock_guard<std::mutex> lock(Mutex);
			if (Registry[aIdentifier].Handler)
			{
				std::thread([aIdentifier]()
					{
						Registry[aIdentifier].Handler(aIdentifier.c_str());
					}).detach();
				called = true;
			}
		}

		return called;
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		{
			const std::lock_guard<std::mutex> lock(Mutex);
			for (auto& [identifier, activekb] : Registry)
			{
				if (activekb.Handler >= aStartAddress && activekb.Handler <= aEndAddress)
				{
					activekb.Handler = nullptr;
					refCounter++;
				}
			}
		}

		return refCounter;
	}
}
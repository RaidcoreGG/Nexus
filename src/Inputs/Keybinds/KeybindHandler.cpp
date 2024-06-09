///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  KeybindHandler.cpp
/// Description  :  Provides functions for keybinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------
/// 
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
	void ADDONAPI_RegisterWithString(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const char* aKeybind)
	{
		Register(aIdentifier, EKeybindHandlerType::DownOnly, aKeybindHandler, aKeybind);
	}

	void ADDONAPI_RegisterWithStruct(const char* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, Keybind aKeybind)
	{
		Register(aIdentifier, EKeybindHandlerType::DownOnly, aKeybindHandler, aKeybind);
	}

	void ADDONAPI_RegisterWithString2(const char* aIdentifier, KEYBINDS_PROCESS2 aKeybindHandler, const char* aKeybind)
	{
		Register(aIdentifier, EKeybindHandlerType::DownAndRelease, aKeybindHandler, aKeybind);
	}

	void ADDONAPI_RegisterWithStruct2(const char* aIdentifier, KEYBINDS_PROCESS2 aKeybindHandler, Keybind aKeybind)
	{
		Register(aIdentifier, EKeybindHandlerType::DownAndRelease, aKeybindHandler, aKeybind);
	}

	void ADDONAPI_InvokeKeybind(const char* aIdentifier, bool aIsRelease)
	{
		Invoke(aIdentifier, aIsRelease);
	}
}

namespace Keybinds
{
	std::mutex								Mutex;
	std::map<std::string, ActiveKeybind>	Registry;

	bool									IsSettingKeybind = false;
	Keybind									CurrentKeybind;
	std::string								CurrentKeybindUsedBy;

	bool									AltTracked;
	bool									CtrlTracked;
	bool									ShiftTracked;
	std::vector<unsigned short>				KeysTracked;
	std::map<std::string, ActiveKeybind>	HeldKeybinds;

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
		Keybind kb{};

		switch (uMsg)
		{
		case WM_ACTIVATE:
			AltTracked = false;
			CtrlTracked = false;
			ShiftTracked = false;
			KeysTracked.clear();

			for (auto& bind : HeldKeybinds)
			{
				Invoke(bind.first, true);
			}

			HeldKeybinds.clear();
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (wParam > 255) break;

			KeyLParam keylp = LParamToKMF(lParam);

			kb.Alt = GetKeyState(VK_MENU) & 0x8000;
			kb.Ctrl = GetKeyState(VK_CONTROL) & 0x8000;
			kb.Shift = GetKeyState(VK_SHIFT) & 0x8000;
			kb.Key = keylp.GetScanCode();

			// if shift, ctrl or alt set key to 0
			if (wParam == 16 || wParam == 17 || wParam == 18)
			{
				kb.Key = 0;
			}

			if (kb == Keybind{})
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
						/* keybind was found */

						/* track these keys for the release event */
						if (kb.Alt) { AltTracked = true; }
						if (kb.Ctrl) { CtrlTracked = true; }
						if (kb.Shift) { ShiftTracked = true; }
						if (std::find(KeysTracked.begin(), KeysTracked.end(), kb.Key) == KeysTracked.end())
						{
							KeysTracked.push_back(kb.Key);
						}

						/* track the actual bind/id combo too */
						if (HeldKeybinds.find(it.first) == HeldKeybinds.end())
						{
							HeldKeybinds[it.first] = it.second;
						}

						if (it.first == KB_TOGGLEHIDEUI)
						{
							// invoke but do not return, pass through to game (multi hide)
							Invoke(it.first);
							break;
						}
						else
						{
							// if Invoke returns true, pass 0 to the wndproc to stop processing
							return Invoke(it.first) ? 0 : 1;
						}
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
			if (wParam > 255) break;

			/* only check if not currently setting keybind */
			if (IsSettingKeybind)
			{
				return uMsg;
			}

			std::vector<std::string> heldBindsPop;

			for (auto& bind : HeldKeybinds)
			{
				if (wParam == 16 && bind.second.Bind.Shift)
				{
					Invoke(bind.first, true);
					ShiftTracked = false;
					heldBindsPop.push_back(bind.first);
				}
				else if (wParam == 17 && bind.second.Bind.Ctrl)
				{
					Invoke(bind.first, true);
					CtrlTracked = false;
					heldBindsPop.push_back(bind.first);
				}
				else if (wParam == 18 && bind.second.Bind.Alt)
				{
					Invoke(bind.first, true);
					AltTracked = false;
					heldBindsPop.push_back(bind.first);
				}
				else
				{
					KeyLParam keylp = LParamToKMF(lParam);
					unsigned short scanCode = keylp.GetScanCode();

					if (scanCode == bind.second.Bind.Key)
					{
						Invoke(bind.first, true);
						KeysTracked.erase(std::find(KeysTracked.begin(), KeysTracked.end(), scanCode));
						heldBindsPop.push_back(bind.first);
					}
				}
			}

			for (auto bind : heldBindsPop)
			{
				HeldKeybinds.erase(bind);
			}

			break;
		}

		return uMsg;
	}

	void Load()
	{
		if (!std::filesystem::exists(Path::F_KEYBINDS)) { return; }

		const std::lock_guard<std::mutex> lock(Mutex);
		
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

	void Save()
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		
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

	void Register(const char* aIdentifier, EKeybindHandlerType aKeybindHandlerType, void* aKeybindHandler, const char* aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);
		Register(aIdentifier, aKeybindHandlerType, aKeybindHandler, requestedBind);
	}
	void Register(const char* aIdentifier, EKeybindHandlerType aKeybindHandlerType, void* aKeybindHandler, Keybind aKeybind)
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
			/* explicitly scoping here because the subsequent Save call will also lock the mutex */
			const std::lock_guard<std::mutex> lock(Mutex);

			/* check if this keybind is not already set */
			if (Registry.find(str) == Registry.end())
			{
				Registry[str].Bind = requestedBind;
			}

			Registry[str].HandlerType = aKeybindHandlerType;
			Registry[str].Handler = aKeybindHandler;
		}

		Save();
	}
	void Deregister(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		{
			/* explicitly scoping here because the subsequent Save call will also lock the mutex */
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

		aKeybind = String::ToUpper(aKeybind);
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

	bool Invoke(std::string aIdentifier, bool aIsRelease)
	{
		bool called = false;

		const std::lock_guard<std::mutex> lock(Mutex);

		auto& bind = Registry[aIdentifier];

		void* handlerFunc = bind.Handler;
		EKeybindHandlerType type = bind.HandlerType;

		if (handlerFunc)
		{
			if ((bind.HandlerType == EKeybindHandlerType::DownOnly && !aIsRelease) ||
				bind.HandlerType == EKeybindHandlerType::DownAndRelease)
			{
				std::thread([aIdentifier, type, handlerFunc, aIsRelease]()
					{
						switch (type)
						{
						case EKeybindHandlerType::DownOnly:
							((KEYBINDS_PROCESS)handlerFunc)(aIdentifier.c_str());
							break;
						case EKeybindHandlerType::DownAndRelease:
							((KEYBINDS_PROCESS2)handlerFunc)(aIdentifier.c_str(), aIsRelease);
							break;
						}
					}).detach();

				called = true;
			}
		}

		return called;
	}

	void Delete(std::string aIdentifier)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		Registry.erase(aIdentifier);
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		const std::lock_guard<std::mutex> lock(Mutex);

		for (auto& [identifier, activekb] : Registry)
		{
			if (activekb.Handler >= aStartAddress && activekb.Handler <= aEndAddress)
			{
				activekb.Handler = nullptr;
				refCounter++;
			}
		}

		return refCounter;
	}
}

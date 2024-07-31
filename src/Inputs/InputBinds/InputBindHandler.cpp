///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  InputBindHandler.cpp
/// Description  :  Provides functions for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "InputBindHandler.h"

#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <string>

#include "Consts.h"
#include "Index.h"
#include "State.h"

#include "Util/Strings.h"
#include "Util/Inputs.h"

/* FIXME: remove this -> DI */
#include "Shared.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace InputBinds
{
	void ADDONAPI_RegisterWithString(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind)
	{
		InputBindApi->Register(aIdentifier, EInputBindHandlerType::DownOnly, aInputBindHandler, aInputBind);
	}

	void ADDONAPI_RegisterWithStruct(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, LegacyInputBind aInputBind)
	{
		InputBindApi->Register(aIdentifier, EInputBindHandlerType::DownOnly, aInputBindHandler, aInputBind);
	}

	void ADDONAPI_RegisterWithString2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind)
	{
		InputBindApi->Register(aIdentifier, EInputBindHandlerType::DownAndRelease, aInputBindHandler, aInputBind);
	}

	void ADDONAPI_RegisterWithStruct2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, LegacyInputBind aInputBind)
	{
		InputBindApi->Register(aIdentifier, EInputBindHandlerType::DownAndRelease, aInputBindHandler, aInputBind);
	}

	void ADDONAPI_InvokeInputBind(const char* aIdentifier, bool aIsRelease)
	{
		InputBindApi->Invoke(aIdentifier, aIsRelease);
	}

	void ADDONAPI_Deregister(const char* aIdentifier)
	{
		InputBindApi->Deregister(aIdentifier);
	}
}

static bool IsLookupTableBuilt = false;
static std::map<unsigned short, std::string> ScancodeLookupTable;

void BuildscanCodeLookupTable()
{
	if (IsLookupTableBuilt) { return; }

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
	};
}

std::string CInputBindApi::ScancodeToString(unsigned short aScanCode)
{
	if (!IsLookupTableBuilt)
	{
		BuildscanCodeLookupTable();
	}

	auto it = ScancodeLookupTable.find(aScanCode);
	if (it != ScancodeLookupTable.end())
	{
		return it->second;
	}

	return "";
}

InputBind CInputBindApi::KBFromString(std::string aInputBind)
{
	if (!IsLookupTableBuilt)
	{
		BuildscanCodeLookupTable();
	}

	InputBind kb{};

	if (aInputBind == "(null)" || aInputBind == "(NULL)") { return kb; }

	aInputBind = String::ToUpper(aInputBind);
	std::string delimiter = "+";

	size_t pos = 0;
	std::string token;
	while ((pos = aInputBind.find(delimiter)) != std::string::npos)
	{
		token = aInputBind.substr(0, pos);
		aInputBind.erase(0, pos + delimiter.length());

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
		if (it->second == aInputBind)
		{
			kb.Code = it->first;
			break;
		}
	}

	return kb;
}

std::string CInputBindApi::KBToString(InputBind aInputBind, bool aPadded)
{
	if (!IsLookupTableBuilt)
	{
		BuildscanCodeLookupTable();
	}

	/* type is not set -> invalid*/
	/* type is keyboard and code 0 because only modifiers -> valid */
	/* type is mouse and code is 0 -> invalid */
	/* type is keyboard, no code, no modifiers -> invalid */
	if (aInputBind.Type == EInputBindType::None ||
		(aInputBind.Type == EInputBindType::Mouse && !aInputBind.Code) ||
		(aInputBind.Type == EInputBindType::Keyboard && !aInputBind.Code && !aInputBind.Alt && !aInputBind.Ctrl && !aInputBind.Shift))
	{
		return "(null)";
	}

	char* buff = new char[100];
	std::string str;

	if (aInputBind.Alt)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_MENU, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	if (aInputBind.Ctrl)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	if (aInputBind.Shift)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	UINT vk = MapVirtualKeyA(aInputBind.Code, MAPVK_VSC_TO_VK);

	if (vk >= 65 && vk <= 90 || vk >= 48 && vk <= 57)
	{
		GetKeyNameTextA(aInputBind.Code << 16, buff, 100);
		str.append(buff);
	}
	else
	{
		if (aInputBind.Type == EInputBindType::Keyboard && aInputBind.Code == 0)
		{
			str = str.substr(0, str.length() - (aPadded ? 3 : 1));
		}
		else
		{
			auto it = ScancodeLookupTable.find(aInputBind.Code);
			if (it != ScancodeLookupTable.end())
			{
				str.append(it->second);
			}
		}
	}

	delete[] buff;

	str = String::ToUpper(str);

	return String::ConvertMBToUTF8(str);
}

CInputBindApi::CInputBindApi(CLogHandler* aLogger)
{
	this->Logger = aLogger;

	this->Load();
}

UINT CInputBindApi::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	InputBind ib{};

	switch (uMsg)
	{
		case WM_ACTIVATEAPP:
		{
			this->ReleaseAll();
			break;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (wParam > 255) break;

			KeyLParam keylp = LParamToKMF(lParam);

			if (wParam == VK_MENU) { this->IsAltHeld = true; }
			else if (wParam == VK_CONTROL) { this->IsCtrlHeld = true; }
			else if (wParam == VK_SHIFT) { this->IsShiftHeld = true; }

			ib.Alt = this->IsAltHeld;
			ib.Ctrl = this->IsCtrlHeld;
			ib.Shift = this->IsShiftHeld;

			ib.Type = EInputBindType::Keyboard; // type keyboard, since we're listening to keypresses
			ib.Code = keylp.GetScanCode();

			// if shift, ctrl or alt set key to 0
			if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
			{
				ib.Code = 0;
			}

			if (ib == InputBind{})
			{
				return uMsg;
			}

			if (keylp.PreviousKeyState)
			{
				return uMsg;
			}

			if (this->IsCapturing)
			{
				/* store the currently held bind */
				this->CapturedInputBind = ib;

				/* prevent invoking InputBinds */
				break;
			}

			auto heldBind = std::find_if(this->Registry.begin(), this->Registry.end(), [ib](auto& activeInputBind) { return activeInputBind.second.Bind == ib; });

			if (heldBind == this->Registry.end()) /* if held bind does not match any registered */
			{
				break;
			}

			if (std::find(this->HeldKeys.begin(), this->HeldKeys.end(), ib.Code) == this->HeldKeys.end())
			{
				this->HeldKeys.push_back(ib.Code);
			}

			/* track the actual bind/id combo too */
			if (this->HeldInputBinds.find(heldBind->first) == this->HeldInputBinds.end())
			{
				this->HeldInputBinds[heldBind->first] = heldBind->second;
			}

			if (heldBind->first == KB_TOGGLEHIDEUI)
			{
				// invoke but do not return, pass through to game (multi hide)
				this->Invoke(heldBind->first);
				break;
			}
			else
			{
				// if Invoke returns true, pass 0 to the wndproc to stop processing
				return this->Invoke(heldBind->first) ? 0 : 1;
			}

			break;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			if (wParam > 255) break;

			if (wParam == VK_MENU) { this->IsAltHeld = false; }
			else if (wParam == VK_CONTROL) { this->IsCtrlHeld = false; }
			else if (wParam == VK_SHIFT) { this->IsShiftHeld = false; }

			/* only check if not currently setting InputBind */
			if (this->IsCapturing)
			{
				return uMsg;
			}

			std::vector<std::string> heldBindsPop;

			//const std::lock_guard<std::mutex> lock(this->MutexHeldKeys);
			for (auto& bind : this->HeldInputBinds)
			{
				if (wParam == VK_SHIFT && bind.second.Bind.Shift)
				{
					this->Invoke(bind.first, true);
					this->IsShiftHeld = false;
					heldBindsPop.push_back(bind.first);
				}
				else if (wParam == VK_CONTROL && bind.second.Bind.Ctrl)
				{
					this->Invoke(bind.first, true);
					this->IsCtrlHeld = false;
					heldBindsPop.push_back(bind.first);
				}
				else if (wParam == VK_MENU && bind.second.Bind.Alt)
				{
					this->Invoke(bind.first, true);
					this->IsAltHeld = false;
					heldBindsPop.push_back(bind.first);
				}
				else
				{
					KeyLParam keylp = LParamToKMF(lParam);
					unsigned short scanCode = keylp.GetScanCode();

					if (scanCode == bind.second.Bind.Code)
					{
						this->Invoke(bind.first, true);
						this->HeldKeys.erase(std::find(this->HeldKeys.begin(), this->HeldKeys.end(), scanCode));
						heldBindsPop.push_back(bind.first);
					}
				}
			}

			for (auto bind : heldBindsPop)
			{
				this->HeldInputBinds.erase(bind);
			}

			break;
		}
	}

	// don't pass keys to game if currently editing InputBinds
	if (this->IsCapturing)
	{
		if (uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP)
		{
			return 0;
		}
	}

	return uMsg;
}

void CInputBindApi::Register(const char* aIdentifier, EInputBindHandlerType aInputBindHandlerType, void* aInputBindHandler, const char* aInputBind)
{
	InputBind requestedBind = CInputBindApi::KBFromString(aInputBind);
	this->Register(aIdentifier, aInputBindHandlerType, aInputBindHandler, requestedBind);
}

void CInputBindApi::Register(const char* aIdentifier, EInputBindHandlerType aInputBindHandlerType, void* aInputBindHandler, InputBind aInputBind)
{
	std::string str = aIdentifier;

	InputBind requestedBind = aInputBind;

	/* check if another identifier, already uses the InputBind */
	std::string res = this->IsInUse(requestedBind);

	if (res != str && res != "")
	{
		/* another identifier uses the same combination */
		requestedBind = {};
	}

	{
		/* explicitly scoping here because the subsequent Save call will also lock the mutex */
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(str);

		/* check if this InputBind is not already set */
		if (it == Registry.end())
		{
			this->Registry[str] = ManagedInputBind{
				requestedBind,
				aInputBindHandlerType,
				aInputBindHandler
			};
		}
		else
		{
			it->second.HandlerType = aInputBindHandlerType;
			it->second.Handler = aInputBindHandler;
		}
	}

	this->Save();
}

void CInputBindApi::Deregister(const char* aIdentifier)
{
	std::string str = aIdentifier;

	{
		/* explicitly scoping here because the subsequent Save call will also lock the mutex */
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(str);
		if (it != this->Registry.end())
		{
			it->second.Handler = nullptr;
		}
	}

	this->Save();
}

std::string CInputBindApi::IsInUse(InputBind aInputBind)
{
	/* sanity check */
	if (aInputBind == InputBind{}) { return ""; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* check if another identifier, already uses the InputBind */
	for (auto& [identifier, InputBind] : this->Registry)
	{
		if (InputBind.Bind == aInputBind)
		{
			return identifier;
		}
	}

	return "";
}

void CInputBindApi::Set(std::string aIdentifier, InputBind aInputBind)
{
	std::string res = this->IsInUse(aInputBind);

	if (res != aIdentifier && res != "") { return; }

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->Registry[aIdentifier].Bind = aInputBind;
	}

	this->Save();
}

bool CInputBindApi::Invoke(std::string aIdentifier, bool aIsRelease)
{
	bool called = false;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto& bind = this->Registry[aIdentifier];

	void* handlerFunc = bind.Handler;
	EInputBindHandlerType type = bind.HandlerType;

	if (handlerFunc)
	{
		if ((bind.HandlerType == EInputBindHandlerType::DownOnly && !aIsRelease) ||
			bind.HandlerType == EInputBindHandlerType::DownAndRelease)
		{
			std::thread([aIdentifier, type, handlerFunc, aIsRelease]()
			{
				switch (type)
				{
					case EInputBindHandlerType::DownOnly:
						((INPUTBINDS_PROCESS)handlerFunc)(aIdentifier.c_str());
						break;
					case EInputBindHandlerType::DownAndRelease:
						((INPUTBINDS_PROCESS2)handlerFunc)(aIdentifier.c_str(), aIsRelease);
						break;
				}
			}).detach();

			called = true;
		}
	}

	return called;
}

void CInputBindApi::Delete(std::string aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.erase(aIdentifier);
}

int CInputBindApi::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, activekb] : this->Registry)
	{
		if (activekb.Handler >= aStartAddress && activekb.Handler <= aEndAddress)
		{
			activekb.Handler = nullptr;
			refCounter++;
		}
	}

	return refCounter;
}

std::map<std::string, ManagedInputBind> CInputBindApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

InputBind CInputBindApi::GetCapturedInputBind() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->CapturedInputBind;
}

void CInputBindApi::StartCapturing()
{
	if (!this->IsCapturing)
	{
		this->ReleaseAll();
	}

	this->IsCapturing = true;
}

void CInputBindApi::EndCapturing()
{
	this->IsCapturing = false;

	this->ReleaseAll();

	CapturedInputBind = InputBind{};
}

void CInputBindApi::Load()
{
	if (!std::filesystem::exists(Index::F_INPUTBINDS)) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	try
	{
		std::ifstream file(Index::F_INPUTBINDS);

		json inputBinds = json::parse(file);

		for (json binding : inputBinds)
		{
			/* if bind is null, or neither key nor code are set -> invalid */
			if (binding.is_null() || (binding["Key"].is_null() && binding["Code"].is_null()))
			{
				continue;
			}

			InputBind ib{};
			binding["Alt"].get_to(ib.Alt);
			binding["Ctrl"].get_to(ib.Ctrl);
			binding["Shift"].get_to(ib.Shift);

			/* neither code nor type null -> inputbind */
			if (!binding["Type"].is_null() && !binding["Code"].is_null())
			{
				binding["Type"].get_to(ib.Type);
				binding["Code"].get_to(ib.Code);
			}
			else /* legacy inputbind */
			{
				ib.Type = EInputBindType::Keyboard;
				binding["Key"].get_to(ib.Code);
			}

			std::string identifier = binding["Identifier"].get<std::string>();

			this->Registry[identifier].Bind = ib;
		}

		file.close();
	}
	catch (json::parse_error& ex)
	{
		Logger->Warning(CH_INPUTBINDS, "InputBinds.json could not be parsed. Error: %s", ex.what());
	}
}

void CInputBindApi::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	json InputBinds = json::array();

	for (auto& it : this->Registry)
	{
		std::string id = it.first;
		InputBind ib = it.second.Bind;

		json binding =
		{
			{"Identifier",	id},
			{"Alt",			ib.Alt},
			{"Ctrl",		ib.Ctrl},
			{"Shift",		ib.Shift},
			{"Type",		ib.Type},
			{"Code",		ib.Code}
		};

		if (!ib.Alt && !ib.Ctrl && !ib.Shift && !ib.Code)
		{
			ib.Type = EInputBindType::None;
		}

		InputBinds.push_back(binding);
	}

	std::ofstream file(Index::F_INPUTBINDS);

	file << InputBinds.dump(1, '\t') << std::endl;

	file.close();
}

void CInputBindApi::ReleaseAll()
{
	this->IsAltHeld = false;
	this->IsCtrlHeld = false;
	this->IsShiftHeld = false;

	//const std::lock_guard<std::mutex> lock(this->MutexHeldKeys);
	for (auto& bind : this->HeldInputBinds)
	{
		this->Invoke(bind.first, true);
	}

	this->HeldKeys.clear();
	this->HeldInputBinds.clear();
}
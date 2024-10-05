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

InputBind CInputBindApi::IBFromString(std::string aInputBind)
{
	if (!IsLookupTableBuilt)
	{
		BuildscanCodeLookupTable();
	}

	InputBind ib{};

	if (String::ToLower(aInputBind) == NULLSTR) { return ib; }

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
			ib.Alt = true;
		}
		else if (token == "CTRL")
		{
			ib.Ctrl = true;
		}
		else if (token == "SHIFT")
		{
			ib.Shift = true;
		}
	}

	if (aInputBind == "LMB")
	{
		ib.Type = EInputBindType::Mouse;
		ib.Code = (unsigned short)EMouseButtons::LMB;
	}
	else if (aInputBind == "RMB")
	{
		ib.Type = EInputBindType::Mouse;
		ib.Code = (unsigned short)EMouseButtons::RMB;
	}
	else if (aInputBind == "MMB")
	{
		ib.Type = EInputBindType::Mouse;
		ib.Code = (unsigned short)EMouseButtons::MMB;
	}
	else if (aInputBind == "M4")
	{
		ib.Type = EInputBindType::Mouse;
		ib.Code = (unsigned short)EMouseButtons::M4;
	}
	else if (aInputBind == "M5")
	{
		ib.Type = EInputBindType::Mouse;
		ib.Code = (unsigned short)EMouseButtons::M5;
	}
	else
	{
		ib.Type = EInputBindType::Keyboard;
		for (auto it = ScancodeLookupTable.begin(); it != ScancodeLookupTable.end(); ++it)
		{
			if (it->second == aInputBind)
			{
				ib.Code = it->first;
				break;
			}
		}
	}

	return ib;
}

std::string CInputBindApi::IBToString(InputBind aInputBind, bool aPadded)
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
		return NULLSTR;
	}

	char* buff = new char[100];
	std::string str;

	if (aInputBind.Alt)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_MENU, MAPVK_VK_TO_VSC_EX) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	if (aInputBind.Ctrl)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC_EX) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	if (aInputBind.Shift)
	{
		GetKeyNameTextA(MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC_EX) << 16, buff, 100);
		str.append(buff);
		str.append(aPadded ? " + " : "+");
	}

	if (aInputBind.Type == EInputBindType::Keyboard)
	{
		UINT vk = MapVirtualKeyA(aInputBind.Code, MAPVK_VSC_TO_VK_EX);

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
	}
	else if (aInputBind.Type == EInputBindType::Mouse)
	{
		switch ((EMouseButtons)aInputBind.Code)
		{
			case EMouseButtons::LMB:
			{
				str.append("LMB");
				break;
			}
			case EMouseButtons::RMB:
			{
				str.append("RMB");
				break;
			}
			case EMouseButtons::MMB:
			{
				str.append("MMB");
				break;
			}
			case EMouseButtons::M4:
			{
				str.append("M4");
				break;
			}
			case EMouseButtons::M5:
			{
				str.append("M5");
				break;
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
	bool isModifier = false;

	/* preprocess for modifiers */
	switch (uMsg)
	{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (wParam == VK_MENU)
			{
				isModifier = true;
				this->IsAltHeld = true;
			}
			else if (wParam == VK_CONTROL)
			{
				isModifier = true;
				this->IsCtrlHeld = true;
			}
			else if (wParam == VK_SHIFT)
			{
				isModifier = true;
				this->IsShiftHeld = true;
			}

			break;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			if (wParam == VK_MENU)
			{
				isModifier = true;
				this->IsAltHeld = false;
			}
			else if (wParam == VK_CONTROL)
			{
				isModifier = true;
				this->IsCtrlHeld = false;
			}
			else if (wParam == VK_SHIFT)
			{
				isModifier = true;
				this->IsShiftHeld = false;
			}

			break;
		}
	}

	InputBind ib{};

	/* actual input processing */
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
			if (wParam > 255) { break; }

			/* if the pressed key is a modifier, reset all modifiers for the actual bind */
			if (isModifier)
			{
				ib.Alt = false;
				ib.Ctrl = false;
				ib.Shift = false;
			}
			else
			{
				ib.Alt = this->IsAltHeld;
				ib.Ctrl = this->IsCtrlHeld;
				ib.Shift = this->IsShiftHeld;
			}

			KeyLParam keylp = LParamToKMF(lParam);
			ib.Type = EInputBindType::Keyboard; // type keyboard, since we're listening to keypresses
			ib.Code = keylp.GetScanCode();

			if (!ib.IsBound())
			{
				break;
			}

			if (keylp.PreviousKeyState)
			{
				break;
			}

			if (this->IsCapturing)
			{
				/* store the currently held bind */
				this->CapturedInputBind = ib;

				/* send zero to prevent invoking InputBinds and further processing */
				return 0;
			}

			/* if something was invoked, send zero to stop further processing */
			if (this->Press(ib))
			{
				return 0;
			}

			break;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			if (wParam > 255) { break; }

			if (isModifier)
			{
				this->Release((unsigned int)wParam);
			}
			else
			{
				this->Release(EInputBindType::Keyboard, LParamToKMF(lParam).GetScanCode());
			}

			/* only check if not currently setting InputBind */
			if (this->IsCapturing)
			{
				/* send non-zero here so key RELEASES get passed through anyway */
				return uMsg;
			}

			break;
		}

		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			ib.Alt = this->IsAltHeld;
			ib.Ctrl = this->IsCtrlHeld;
			ib.Shift = this->IsShiftHeld;

			ib.Type = EInputBindType::Mouse; // type mouse, since we're listening to mbuttons

			if (uMsg == WM_MBUTTONDOWN)
			{
				ib.Code = (unsigned short)EMouseButtons::MMB;
			} /* else it's implicitly an XBUTTON */
			else if (HIWORD(wParam) == XBUTTON1)
			{
				ib.Code = (unsigned short)EMouseButtons::M4;
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				ib.Code = (unsigned short)EMouseButtons::M5;
			}

			if (this->IsCapturing)
			{
				/* store the currently held bind */
				this->CapturedInputBind = ib;

				/* send zero to prevent invoking InputBinds and further processing */
				return 0;
			}

			/* if something was invoked, send zero to stop further processing */
			if (this->Press(ib))
			{
				return 0;
			}

			break;
		}
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			if (uMsg == WM_MBUTTONUP)
			{
				this->Release(EInputBindType::Mouse, (unsigned short)EMouseButtons::MMB);
			} /* else it's implicitly an XBUTTON */
			else if (HIWORD(wParam) == XBUTTON1)
			{
				this->Release(EInputBindType::Mouse, (unsigned short)EMouseButtons::M4);
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				this->Release(EInputBindType::Mouse, (unsigned short)EMouseButtons::M5);
			}

			/* only check if not currently setting InputBind */
			if (this->IsCapturing)
			{
				/* send non-zero here so mouse RELEASES get passed through anyway */
				return uMsg;
			}

			break;
		}
	}

	return uMsg;
}

void CInputBindApi::Register(const char* aIdentifier, EInputBindHandlerType aInputBindHandlerType, void* aInputBindHandler, const char* aInputBind)
{
	InputBind requestedBind = CInputBindApi::IBFromString(aInputBind);
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

		/* override type */
		if (!ib.Code)
		{
			ib.Type = EInputBindType::None;
		}

		InputBinds.push_back(binding);
	}

	std::ofstream file(Index::F_INPUTBINDS);

	file << InputBinds.dump(1, '\t') << std::endl;

	file.close();
}

bool CInputBindApi::Press(const InputBind& aInputBind)
{
	auto heldBind = this->FindInputBind(aInputBind);

	/* if held bind does not match any registered */
	if (heldBind == this->Registry.end())
	{
		return false;
	}

	/* track the actual bind/id combo */
	this->HeldInputBinds[heldBind->first] = heldBind->second;

	bool invoked = this->Invoke(heldBind->first);;

	/* if was invoked -> stop processing (unless it was the togglehideui bind, pass through for multi hide)*/
	if (invoked && heldBind->first != KB_TOGGLEHIDEUI)
	{
		return true;
	}

	return false;
}

void CInputBindApi::Release(unsigned int aModifierVK)
{
	std::vector<std::string> heldBindsPop;

	for (auto& entry : this->HeldInputBinds)
	{
		if (aModifierVK == VK_SHIFT && entry.second.Bind.Shift)
		{
			this->Invoke(entry.first, true);
			heldBindsPop.push_back(entry.first);
		}
		else if (aModifierVK == VK_CONTROL && entry.second.Bind.Ctrl)
		{
			this->Invoke(entry.first, true);
			heldBindsPop.push_back(entry.first);
		}
		else if (aModifierVK == VK_MENU && entry.second.Bind.Alt)
		{
			this->Invoke(entry.first, true);
			heldBindsPop.push_back(entry.first);
		}
	}

	for (auto entry : heldBindsPop)
	{
		this->HeldInputBinds.erase(entry);
	}
}

void CInputBindApi::Release(EInputBindType aType, unsigned short aCode)
{
	std::vector<std::string> heldBindsPop;

	for (auto& entry : this->HeldInputBinds)
	{
		if (entry.second.Bind.Type == aType && entry.second.Bind.Code == aCode)
		{
			this->Invoke(entry.first, true);
			heldBindsPop.push_back(entry.first);
		}
	}

	for (auto entry : heldBindsPop)
	{
		this->HeldInputBinds.erase(entry);
	}
}

void CInputBindApi::ReleaseAll()
{
	this->IsAltHeld = false;
	this->IsCtrlHeld = false;
	this->IsShiftHeld = false;

	for (auto& bind : this->HeldInputBinds)
	{
		this->Invoke(bind.first, true);
	}

	this->HeldInputBinds.clear();
}

std::map<std::string, ManagedInputBind>::iterator CInputBindApi::FindInputBind(const InputBind& aInputBind)
{
	return std::find_if(this->Registry.begin(), this->Registry.end(), [aInputBind](auto& entry)
	{
		return entry.second.Bind == aInputBind;
	});
}

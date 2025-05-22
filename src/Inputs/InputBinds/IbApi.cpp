///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbApi.cpp
/// Description  :  Provides functions for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbApi.h"

#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "Consts.h"
#include "Index.h"
#include "State.h"
#include "Util/Strings.h"
#include "Util/Inputs.h"
#include "IbConst.h"

CInputBindApi::CInputBindApi(CEventApi* aEventApi, CLogHandler* aLogger)
{
	assert(aEventApi);
	assert(aLogger);

	this->EventApi = aEventApi;
	this->Logger = aLogger;

	this->Load();
}

UINT CInputBindApi::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (this->ProcessInput(uMsg, wParam, lParam))
	{
		return 0;
	}

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

			KeystrokeMessageFlags keylp = LParamToKMF(lParam);
			ib.Device = EInputDevice::Keyboard; // type keyboard, since we're listening to keypresses
			ib.Code = keylp.GetScanCode();

			if (!ib.IsBound())
			{
				break;
			}

			if (keylp.PreviousKeyState)
			{
				break;
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
				/* do this, to check all binds that use the modifier */
				this->Release((unsigned int)wParam);
			}
			
			/* always do this, because even if it's a modifier, it might be a modifier-key bind */
			this->Release(EInputDevice::Keyboard, LParamToKMF(lParam).GetScanCode());

			break;
		}

		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			ib.Alt = this->IsAltHeld;
			ib.Ctrl = this->IsCtrlHeld;
			ib.Shift = this->IsShiftHeld;

			ib.Device = EInputDevice::Mouse; // type mouse, since we're listening to mbuttons

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
				this->Release(EInputDevice::Mouse, (unsigned short)EMouseButtons::MMB);
			} /* else it's implicitly an XBUTTON */
			else if (HIWORD(wParam) == XBUTTON1)
			{
				this->Release(EInputDevice::Mouse, (unsigned short)EMouseButtons::M4);
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				this->Release(EInputDevice::Mouse, (unsigned short)EMouseButtons::M5);
			}

			break;
		}
	}

	return uMsg;
}

void CInputBindApi::Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, const char* aInputBind)
{
	this->Register(aIdentifier, aInputBindHandlerType, aInputBindHandler, IBFromString(aInputBind));
}

void CInputBindApi::Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, InputBind aInputBind)
{
	/* check if another identifier, already uses the InputBind */
	std::string usedBy = this->IsInUse(aInputBind);

	/* another identifier uses the same combination */
	if (!usedBy.empty() && usedBy != aIdentifier)
	{
		aInputBind = {};
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	/* check if this InputBind is not already set */
	if (it == Registry.end())
	{
		IbMapping mapping{};
		mapping.Bind = aInputBind;
		mapping.HandlerType = aInputBindHandlerType;
		switch (aInputBindHandlerType)
		{
			case EIbHandlerType::DownOnly:
			{
				mapping.Handler_DownOnly = (INPUTBINDS_PROCESS)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownAndRelease:
			{
				mapping.Handler_DownAndRelease = (INPUTBINDS_PROCESS2)aInputBindHandler;
				break;
			}
		}

		/* Force handler type to none, if there's no handler. */
		if (!aInputBindHandler)
		{
			mapping.HandlerType = EIbHandlerType::None;
		}

		this->Registry.emplace(aIdentifier, mapping);
	}
	else
	{
		it->second.HandlerType = aInputBindHandlerType;
		switch (aInputBindHandlerType)
		{
			case EIbHandlerType::DownOnly:
			{
				it->second.Handler_DownOnly = (INPUTBINDS_PROCESS)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownAndRelease:
			{
				it->second.Handler_DownAndRelease = (INPUTBINDS_PROCESS2)aInputBindHandler;
				break;
			}
		}

		/* Force handler type to none, if there's no handler. */
		if (!aInputBindHandler)
		{
			it->second.HandlerType = EIbHandlerType::None;
		}
	}

	this->Save();

	/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
	std::thread([this]() {
		this->EventApi->Raise("EV_INPUTBIND_UPDATED");
	}).detach();
}

void CInputBindApi::Deregister(const char* aIdentifier)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(str);
	if (it != this->Registry.end())
	{
		switch (it->second.HandlerType)
		{
			case EIbHandlerType::DownOnly:
			{
				it->second.Handler_DownOnly = nullptr;
				break;
			}
			case EIbHandlerType::DownAndRelease:
			{
				it->second.Handler_DownAndRelease = nullptr;
				break;
			}
		}

		it->second.HandlerType = EIbHandlerType::None;
	}

	this->Save();
}

std::string CInputBindApi::IsInUse(InputBind aInputBind)
{
	if (aInputBind == InputBind{}) { return ""; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aInputBind](auto& entry)
	{
		return entry.second.Bind == aInputBind;
	});

	if (it != this->Registry.end())
	{
		/* return the identifier that's already using this combination */
		return it->first;
	}

	return "";
}

bool CInputBindApi::HasHandler(const std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		switch (it->second.HandlerType)
		{
			case EIbHandlerType::DownOnly:
			{
				return it->second.Handler_DownOnly != nullptr;
				break;
			}
			case EIbHandlerType::DownAndRelease:
			{
				return it->second.Handler_DownAndRelease != nullptr;
				break;
			}
		}
	}
	
	return false;
}

const InputBind& CInputBindApi::Get(const std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		return it->second.Bind;
	}

	return {};
}

void CInputBindApi::Set(std::string aIdentifier, InputBind aInputBind)
{
	/* check if another identifier, already uses the InputBind */
	std::string usedBy = this->IsInUse(aInputBind);

	/* another identifier uses the same combination */
	if (!usedBy.empty() && usedBy != aIdentifier)
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		it->second.Bind = aInputBind;
	}
	else
	{
		IbMapping mapping{};
		mapping.Bind = aInputBind;
		this->Registry.emplace(aIdentifier, mapping);
	}

	this->Save();

	/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
	std::thread([this]() {
		this->EventApi->Raise("EV_INPUTBIND_UPDATED");
	}).detach();
}

bool CInputBindApi::Invoke(std::string aIdentifier, bool aIsRelease)
{
	bool called = false;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it == this->Registry.end())
	{
		return false;
	}

	switch (it->second.HandlerType)
	{
		default:
		case EIbHandlerType::None:
		{
			return false;
		}
		case EIbHandlerType::DownOnly:
		{
			if (!aIsRelease)
			{
				INPUTBINDS_PROCESS handler = it->second.Handler_DownOnly;

				/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
				std::thread([aIdentifier, handler]()
				{
					handler(aIdentifier.c_str());
				}).detach();

				return true;
			}

			return false;
		}
		case EIbHandlerType::DownAndRelease:
		{
			INPUTBINDS_PROCESS2 handler = it->second.Handler_DownAndRelease;

			/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
			std::thread([aIdentifier, handler, aIsRelease]()
			{
				handler(aIdentifier.c_str(), aIsRelease);
			}).detach();

			return true;
		}
	}
}

void CInputBindApi::Delete(std::string aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.erase(aIdentifier);

	/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
	std::thread([this]() {
		this->EventApi->Raise("EV_INPUTBIND_UPDATED");
	}).detach();
}

int CInputBindApi::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, mapping] : this->Registry)
	{
		switch (mapping.HandlerType)
		{
			case EIbHandlerType::DownOnly:
			{
				if (mapping.Handler_DownOnly >= aStartAddress && mapping.Handler_DownOnly <= aEndAddress)
				{
					mapping.Handler_DownOnly = nullptr;
					refCounter++;
				}
				break;
			}
			case EIbHandlerType::DownAndRelease:
			{
				if (mapping.Handler_DownAndRelease >= aStartAddress && mapping.Handler_DownAndRelease <= aEndAddress)
				{
					mapping.Handler_DownAndRelease = nullptr;
					refCounter++;
				}
				break;
			}
		}
	}

	return refCounter;
}

std::map<std::string, IbMapping> CInputBindApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

void CInputBindApi::LoadSafe()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Load();
}

void CInputBindApi::Load()
{
	if (!std::filesystem::exists(Index::F_INPUTBINDS)) { return; }

	try
	{
		std::ifstream file(Index::F_INPUTBINDS);

		json inputBinds = json::parse(file);

		for (json binding : inputBinds)
		{
			/* if bind is null, or neither key nor code are set -> invalid */
			if (binding.is_null())                                     { continue; }
			if (binding["Identifier"].is_null())                       { continue; }
			if (binding["Key"].is_null() && binding["Code"].is_null()) { continue; }

			InputBind ib{};
			ib.Alt   = binding.value("Alt",   false);
			ib.Ctrl  = binding.value("Ctrl",  false);
			ib.Shift = binding.value("Shift", false);

			/* neither code nor type null -> inputbind */
			if (!binding["Type"].is_null() && !binding["Code"].is_null())
			{
				binding["Type"].get_to(ib.Device);
				binding["Code"].get_to(ib.Code);
			}
			else if (!binding["Key"].is_null()) /* legacy inputbind */
			{
				ib.Device = EInputDevice::Keyboard;
				binding["Key"].get_to(ib.Code);
			}
			else
			{
				/* Should be unreachable. */
				continue;
			}

			if (ib.Code == 0)
			{
				ib.Device = EInputDevice::None;
				ib.Alt   = false;
				ib.Ctrl  = false;
				ib.Shift = false;
			}

			std::string identifier = binding["Identifier"].get<std::string>();

			auto it = this->Registry.find(identifier);

			if (it != this->Registry.end())
			{
				it->second.Bind = ib;
			}
			else
			{
				IbMapping mapping{};
				mapping.Bind = ib;
				this->Registry.emplace(identifier, mapping);
			}
		}

		file.close();
	}
	catch (json::parse_error& ex)
	{
		Logger->Warning(CH_INPUTBINDS, "InputBinds.json could not be parsed. Error: %s", ex.what());
	}
}

void CInputBindApi::SaveSafe()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Save();
}

void CInputBindApi::Save()
{
	json bindings = json::array();

	for (auto& [id, ib] : this->Registry)
	{
		json binding =
		{
			{"Identifier", id},
			{"Alt",        ib.Bind.Alt},
			{"Ctrl",       ib.Bind.Ctrl},
			{"Shift",      ib.Bind.Shift},
			{"Type",       ib.Bind.Device},
			{"Code",       ib.Bind.Code}
		};

		/* override type */
		if (!ib.Bind.Code)
		{
			binding["Type"] = EInputDevice::None;
		}

		bindings.push_back(binding);
	}

	std::ofstream file(Index::F_INPUTBINDS);

	file << bindings.dump(1, '\t') << std::endl;

	file.close();
}

bool CInputBindApi::Press(const InputBind& aInputBind)
{
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aInputBind](auto& entry)
	{
		return entry.second.Bind == aInputBind;
	});

	if (it == this->Registry.end())
	{
		/* return the identifier that's already using this combination */
		return false;
	}

	/* track the actual bind/id combo */
	/* FIXME: map lookup/emplacement */
	this->HeldInputBinds[it->first] = it->second;

	bool invoked = this->Invoke(it->first);

	/* if was invoked -> stop processing (unless it was the togglehideui bind, pass through for multi hide)*/
	if (invoked && it->first != "KB_TOGGLEHIDEUI")
	{
		return true;
	}

	return false;
}

void CInputBindApi::Release(unsigned int aModifierVK)
{
	for (auto it = this->HeldInputBinds.begin(); it != this->HeldInputBinds.end();)
	{
		if (aModifierVK == VK_SHIFT && it->second.Bind.Shift)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else if (aModifierVK == VK_CONTROL && it->second.Bind.Ctrl)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else if (aModifierVK == VK_MENU && it->second.Bind.Alt)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CInputBindApi::Release(EInputDevice aDevice, unsigned short aCode)
{
	for (auto it = this->HeldInputBinds.begin(); it != this->HeldInputBinds.end();)
	{
		if (it->second.Bind.Device == aDevice && it->second.Bind.Code == aCode)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CInputBindApi::ReleaseAll()
{
	this->IsAltHeld   = false;
	this->IsCtrlHeld  = false;
	this->IsShiftHeld = false;

	for (auto& bind : this->HeldInputBinds)
	{
		this->Invoke(bind.first, true);
	}

	this->HeldInputBinds.clear();
}

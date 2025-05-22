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

	const InputBind& capture = this->GetCaptureRef();

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

			KeystrokeMessageFlags keylp = LParamToKMF(lParam);

			/* Do not listen to repeated keydowns. */
			if (keylp.PreviousKeyState)
			{
				break;
			}

			/* Do not dispatch null binds. */
			if (!capture.IsBound())
			{
				break;
			}

			/* if something was invoked, send zero to stop further processing */
			if (this->Press(capture))
			{
				return 0;
			}

			break;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			if (wParam > 255) { break; }

			/* Additionally send a modifier release, if it is one. */
			if (wParam == VK_MENU || wParam == VK_CONTROL || wParam == VK_SHIFT)
			{
				this->Release((unsigned int)wParam);
			}
			
			/* Release binds matching the passed key. Standalone modifier binds as well. */
			this->Release(EInputDevice::Keyboard, LParamToKMF(lParam).GetScanCode());

			break;
		}

		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			/* if something was invoked, send zero to stop further processing */
			if (this->Press(capture))
			{
				return 0;
			}

			break;
		}
		case WM_MBUTTONUP:
		{
			this->Release(EInputDevice::Mouse, (unsigned short)EMouseButtons::MMB);
			break;
		}
		case WM_XBUTTONUP:
		{
			if (HIWORD(wParam) == XBUTTON1)
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

void CInputBindApi::SetPassthrough(std::string aIdentifier, bool aPassthrough)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it == this->Registry.end())
	{
		return;
	}
	
	it->second.Bind.Passthrough = aPassthrough;

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
			ib.Alt         = binding.value("Alt",         false);
			ib.Ctrl        = binding.value("Ctrl",        false);
			ib.Shift       = binding.value("Shift",       false);
			ib.Passthrough = binding.value("Passthrough", false);

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
			{"Identifier",  id                 },
			{"Alt",         ib.Bind.Alt        },
			{"Ctrl",        ib.Bind.Ctrl       },
			{"Shift",       ib.Bind.Shift      },
			{"Type",        ib.Bind.Device     },
			{"Code",        ib.Bind.Code       },
			{"Passthrough", ib.Bind.Passthrough}
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
		return false;
	}

	auto heldIt = this->HeldInputBinds.find(it->first);

	/* track the actual bind/id combo */
	if (heldIt == this->HeldInputBinds.end())
	{
		this->HeldInputBinds.emplace(it->first, it->second);
	}

	bool invoked = this->Invoke(it->first);

	/* if was invoked -> stop processing (unless it was the togglehideui bind, pass through for multi hide)*/
	if (invoked && it->second.Bind.Passthrough == false)
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
	for (auto& bind : this->HeldInputBinds)
	{
		this->Invoke(bind.first, true);
	}

	this->HeldInputBinds.clear();
}

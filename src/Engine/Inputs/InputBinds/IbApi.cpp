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

#include "Engine/Index/Index.h"
#include "Util/Strings.h"
#include "Util/Inputs.h"
#include "IbConst.h"

CInputBindApi::CInputBindApi(CEventApi* aEventApi, CLogApi* aLogger)
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

	const InputBind_t& capture = this->GetCaptureRef();

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

void CInputBindApi::Register(const char* aIdentifier, EIbHandlerType aInputBindHandlerType, void* aInputBindHandler, InputBind_t aInputBind)
{
	/* check if another identifier, already uses the InputBind_t */
	std::string usedBy = this->IsInUse(aInputBind);

	/* another identifier uses the same combination */
	if (!usedBy.empty() && usedBy != aIdentifier)
	{
		aInputBind = {};
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	/* check if this InputBind_t is not already set */
	if (it == Registry.end())
	{
		IbMapping_t mapping{};
		mapping.Bind = aInputBind;
		mapping.HandlerType = aInputBindHandlerType;
		switch (aInputBindHandlerType)
		{
			case EIbHandlerType::DownAsync:
			{
				mapping.Handler_DownOnlyAsync = (INPUTBINDS_PROCESS)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownReleaseAsync:
			{
				mapping.Handler_DownReleaseAsync = (INPUTBINDS_PROCESS2)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownRelease:
			{
				mapping.Handler_DownRelease = (INPUTBINDS_PROCESS3)aInputBindHandler;
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
			case EIbHandlerType::DownAsync:
			{
				it->second.Handler_DownOnlyAsync = (INPUTBINDS_PROCESS)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownReleaseAsync:
			{
				it->second.Handler_DownReleaseAsync = (INPUTBINDS_PROCESS2)aInputBindHandler;
				break;
			}
			case EIbHandlerType::DownRelease:
			{
				it->second.Handler_DownRelease = (INPUTBINDS_PROCESS3)aInputBindHandler;
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
			case EIbHandlerType::DownAsync:
			{
				it->second.Handler_DownOnlyAsync = nullptr;
				break;
			}
			case EIbHandlerType::DownReleaseAsync:
			{
				it->second.Handler_DownReleaseAsync = nullptr;
				break;
			}
			case EIbHandlerType::DownRelease:
			{
				it->second.Handler_DownRelease = nullptr;
				break;
			}
		}

		it->second.HandlerType = EIbHandlerType::None;
	}

	this->Save();
}

std::string CInputBindApi::IsInUse(InputBind_t aInputBind)
{
	if (aInputBind == InputBind_t{}) { return ""; }

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
			case EIbHandlerType::DownAsync:
			{
				return it->second.Handler_DownOnlyAsync != nullptr;
				break;
			}
			case EIbHandlerType::DownReleaseAsync:
			{
				return it->second.Handler_DownReleaseAsync != nullptr;
				break;
			}
			case EIbHandlerType::DownRelease:
			{
				return it->second.Handler_DownRelease != nullptr;
				break;
			}
		}
	}
	
	return false;
}

const InputBind_t* CInputBindApi::Get(const std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		return &it->second.Bind;
	}

	return nullptr;
}

void CInputBindApi::Set(std::string aIdentifier, InputBind_t aInputBind)
{
	/* check if another identifier, already uses the InputBind_t */
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
		IbMapping_t mapping{};
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
	
	it->second.Passthrough = aPassthrough;

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
		case EIbHandlerType::DownAsync:
		{
			if (!aIsRelease)
			{
				INPUTBINDS_PROCESS handler = it->second.Handler_DownOnlyAsync;

				/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
				std::thread([aIdentifier, handler]()
				{
					handler(aIdentifier.c_str());
				}).detach();

				return true;
			}

			return false;
		}
		case EIbHandlerType::DownReleaseAsync:
		{
			INPUTBINDS_PROCESS2 handler = it->second.Handler_DownReleaseAsync;

			/* FIXME: https://github.com/RaidcoreGG/Nexus/issues/111 */
			std::thread([aIdentifier, handler, aIsRelease]()
			{
				handler(aIdentifier.c_str(), aIsRelease);
			}).detach();

			return true;
		}
		case EIbHandlerType::DownRelease:
		{
			INPUTBINDS_PROCESS3 handler = it->second.Handler_DownRelease;

			return handler(aIdentifier.c_str(), aIsRelease);
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
			case EIbHandlerType::DownAsync:
			{
				if (mapping.Handler_DownOnlyAsync >= aStartAddress && mapping.Handler_DownOnlyAsync <= aEndAddress)
				{
					mapping.Handler_DownOnlyAsync = nullptr;
					mapping.HandlerType = EIbHandlerType::None;
					refCounter++;
				}
				break;
			}
			case EIbHandlerType::DownReleaseAsync:
			{
				if (mapping.Handler_DownReleaseAsync >= aStartAddress && mapping.Handler_DownReleaseAsync <= aEndAddress)
				{
					mapping.Handler_DownReleaseAsync = nullptr;
					mapping.HandlerType = EIbHandlerType::None;
					refCounter++;
				}
				break;
			}
			case EIbHandlerType::DownRelease:
			{
				if (mapping.Handler_DownRelease >= aStartAddress && mapping.Handler_DownRelease <= aEndAddress)
				{
					mapping.Handler_DownRelease = nullptr;
					mapping.HandlerType = EIbHandlerType::None;
					refCounter++;
				}
				break;
			}
		}
	}

	return refCounter;
}

std::map<std::string, IbMapping_t> CInputBindApi::GetRegistry() const
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
	if (!std::filesystem::exists(Index(EPath::InputBinds))) { return; }

	try
	{
		std::ifstream file(Index(EPath::InputBinds));

		json inputBinds = json::parse(file);

		for (json binding : inputBinds)
		{
			/* if bind is null, or neither key nor code are set -> invalid */
			if (binding.is_null())                                     { continue; }
			if (binding["Identifier"].is_null())                       { continue; }
			if (binding["Key"].is_null() && binding["Code"].is_null()) { continue; }

			InputBind_t ib{};
			ib.Alt           = binding.value("Alt",         false);
			ib.Ctrl          = binding.value("Ctrl",        false);
			ib.Shift         = binding.value("Shift",       false);
			bool passthrough = binding.value("Passthrough", false);

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
				it->second.Passthrough = passthrough;
			}
			else
			{
				IbMapping_t mapping{};
				mapping.Bind = ib;
				mapping.Passthrough = passthrough;
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
			{"Passthrough", ib.Passthrough     }
		};

		/* override type */
		if (!ib.Bind.Code)
		{
			binding["Type"] = EInputDevice::None;
		}

		bindings.push_back(binding);
	}

	std::ofstream file(Index(EPath::InputBinds));

	file << bindings.dump(1, '\t') << std::endl;

	file.close();
}

bool CInputBindApi::Press(const InputBind_t& aInputBind)
{
	std::string identifier;
	bool passthrough = false;

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aInputBind](auto& entry)
		{
			return entry.second.Bind == aInputBind;
		});

		if (it == this->Registry.end())
		{
			return false;
		}

		identifier = it->first;
		passthrough = it->second.Passthrough;
	}

	assert(this->HeldInputBinds.find(identifier) == this->HeldInputBinds.end());

	/* track the actual bind/id combo */
	this->HeldInputBinds.emplace(identifier, aInputBind);

	bool invoked = this->Invoke(identifier);

	/* if was invoked -> stop processing (unless it was bind with passthrough flag) */
	if (invoked && passthrough == false)
	{
		return true;
	}

	return false;
}

void CInputBindApi::Release(unsigned int aModifierVK)
{
	for (auto it = this->HeldInputBinds.begin(); it != this->HeldInputBinds.end();)
	{
		if (aModifierVK == VK_SHIFT && it->second.Shift)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else if (aModifierVK == VK_CONTROL && it->second.Ctrl)
		{
			this->Invoke(it->first, true);
			it = this->HeldInputBinds.erase(it);
		}
		else if (aModifierVK == VK_MENU && it->second.Alt)
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
		if (it->second.Device == aDevice && it->second.Code == aCode)
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

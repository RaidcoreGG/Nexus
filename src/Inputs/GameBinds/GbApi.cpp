///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbApi.cpp
/// Description  :  Provides functions for game InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "GbApi.h"

#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;
#include "pugixml/pugixml.hpp"

#include "Context.h"
#include "Index.h"
#include "Util/Inputs.h"
#include "GbConst.h"

void CGameBindsApi::OnUEInputBindChanged(void* aData)
{
	static CGameBindsApi* s_GameBindsApi = nullptr;

	if (!s_GameBindsApi)
	{
		CContext* ctx = CContext::GetContext();
		s_GameBindsApi = ctx->GetGameBindsApi();

		assert(s_GameBindsApi);
	}

	struct UEKey
	{
		unsigned DeviceType; // 0 = Unset, 1 = Mouse, 2 = Keyboard
		signed Code; // Custom ArenaNet Scancode
		signed Modifiers; // Bit 1 = Shfit, Bit 2 = Ctrl, Bit 3 = Alt
	};

	struct UEInputBindChanged
	{
		EGameBinds Identifier;
		unsigned Index; // 0 = Primary, 1 = Secondary
		UEKey Bind;
	};

	UEInputBindChanged* kbChange = (UEInputBindChanged*)aData;

	/* abort if it's the secondary bind, but the key is already set*/
	if (kbChange->Index == 1 && s_GameBindsApi->IsBound(kbChange->Identifier))
	{
		return;
	}

	InputBind ib{};

	/* if key was bound (keyboard) */
	if (kbChange->Bind.DeviceType == 2)
	{
		ib.Alt = kbChange->Bind.Modifiers & 0b0100;
		ib.Ctrl = kbChange->Bind.Modifiers & 0b0010;
		ib.Shift = kbChange->Bind.Modifiers & 0b0001;

		ib.Type = EInputBindType::Keyboard;
		ib.Code = GameScanCodeToScanCode(kbChange->Bind.Code);
	}
	else if (kbChange->Bind.DeviceType == 1) /* (mouse) */
	{
		ib.Alt = kbChange->Bind.Modifiers & 0b0100;
		ib.Ctrl = kbChange->Bind.Modifiers & 0b0010;
		ib.Shift = kbChange->Bind.Modifiers & 0b0001;

		ib.Type = EInputBindType::Mouse;
		if (kbChange->Bind.Code == 0)
		{
			ib.Code = (unsigned short)EMouseButtons::LMB;
		}
		else if (kbChange->Bind.Code == 2)
		{
			ib.Code = (unsigned short)EMouseButtons::RMB;
		}
		else if (kbChange->Bind.Code == 1)
		{
			ib.Code = (unsigned short)EMouseButtons::MMB;
		}
		else if (kbChange->Bind.Code == 3)
		{
			ib.Code = (unsigned short)EMouseButtons::M4;
		}
		else if (kbChange->Bind.Code == 4)
		{
			ib.Code = (unsigned short)EMouseButtons::M5;
		}
	}

	s_GameBindsApi->Set(kbChange->Identifier, ib, true);

	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	uictx->Invalidate();
}

CGameBindsApi::CGameBindsApi(CRawInputApi* aRawInputApi, CLogHandler* aLogger, CEventApi* aEventApi, CLocalization* aLocalization)
{
	assert(aRawInputApi);
	assert(aLogger);
	assert(aEventApi);

	this->RawInputApi = aRawInputApi;
	this->Logger = aLogger;
	this->EventApi = aEventApi;
	this->Language = aLocalization;

	this->Load(Index::F_GAMEBINDS);
	this->AddDefaultBinds();

	this->EventApi->Subscribe(EV_UE_KB_CH, CGameBindsApi::OnUEInputBindChanged);
}

CGameBindsApi::~CGameBindsApi()
{
	this->EventApi->Unsubscribe(EV_UE_KB_CH, CGameBindsApi::OnUEInputBindChanged);
}

void CGameBindsApi::PressAsync(EGameBinds aGameBind)
{
	std::thread([this, aGameBind]() {
		this->Press(aGameBind);
	}).detach();
}

void CGameBindsApi::ReleaseAsync(EGameBinds aGameBind)
{
	std::thread([this, aGameBind]() {
		this->Release(aGameBind);
	}).detach();
}

void CGameBindsApi::InvokeAsync(EGameBinds aGameBind, int aDuration)
{
	std::thread([this, aGameBind, aDuration]() {
		this->Press(aGameBind);
		if (aDuration > 0)
		{
			Sleep(aDuration);
		}
		this->Release(aGameBind);
	}).detach();
}

void CGameBindsApi::Press(EGameBinds aGameBind)
{
	/* Migrate legacy bind that the game merged. */
	if (aGameBind == EGameBinds::LEGACY_MoveSwimUp)
	{
		aGameBind = EGameBinds::MoveJump_SwimUp_FlyUp;
	}

	const InputBind& ib = this->Get(aGameBind);

	if (!ib.IsBound())
	{
		return;
	}

	if (ib.Alt)
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_SYSKEYDOWN,
			VK_MENU,
			GetKeyMessageLPARAM(VK_MENU, true, true)
		);
	}
	else if (!ib.Alt && GetAsyncKeyState(VK_MENU))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_SYSKEYUP,
			VK_MENU,
			GetKeyMessageLPARAM(VK_MENU, false, true)
		);
	}

	if (ib.Ctrl)
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_CONTROL,
			GetKeyMessageLPARAM(VK_CONTROL, true, false)
		);
	}
	else if (!ib.Ctrl && GetAsyncKeyState(VK_CONTROL))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_CONTROL,
			GetKeyMessageLPARAM(VK_CONTROL, false, false)
		);
	}

	if (ib.Shift)
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_SHIFT,
			GetKeyMessageLPARAM(VK_SHIFT, true, false)
		);
	}
	else if (!ib.Shift && GetAsyncKeyState(VK_SHIFT))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_SHIFT,
			GetKeyMessageLPARAM(VK_SHIFT, false, false)
		);
	}

	if (ib.Type == EInputBindType::Keyboard)
	{
		UINT vk = MapVirtualKeyA(ib.Code, MAPVK_VSC_TO_VK_EX);
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			vk,
			GetKeyMessageLPARAM_ScanCode(ib.Code, true, false)
		);
	}
	else if (ib.Type == EInputBindType::Mouse)
	{
		/* get point for lparam */
		POINT point{};
		GetCursorPos(&point);

		switch ((EMouseButtons)ib.Code)
		{
			case EMouseButtons::LMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_LBUTTONDOWN,
					GetMouseMessageWPARAM(EMouseButtons::LMB, ib.Ctrl, ib.Shift, true),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::RMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_RBUTTONDOWN,
					GetMouseMessageWPARAM(EMouseButtons::RMB, ib.Ctrl, ib.Shift, true),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::MMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_MBUTTONDOWN,
					GetMouseMessageWPARAM(EMouseButtons::MMB, ib.Ctrl, ib.Shift, true),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::M4:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_XBUTTONDOWN,
					GetMouseMessageWPARAM(EMouseButtons::M4, ib.Ctrl, ib.Shift, true),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::M5:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_XBUTTONDOWN,
					GetMouseMessageWPARAM(EMouseButtons::M5, ib.Ctrl, ib.Shift, true),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
		}
	}

	/* restore modifiers */
	if (!ib.Alt && GetAsyncKeyState(VK_MENU))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_SYSKEYDOWN,
			VK_MENU,
			GetKeyMessageLPARAM(VK_MENU, true, true)
		);
	}
	if (!ib.Ctrl && GetAsyncKeyState(VK_CONTROL))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_CONTROL,
			GetKeyMessageLPARAM(VK_CONTROL, true, false)
		);
	}
	if (!ib.Shift && GetAsyncKeyState(VK_SHIFT))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_SHIFT,
			GetKeyMessageLPARAM(VK_SHIFT, true, false)
		);
	}
}

void CGameBindsApi::Release(EGameBinds aGameBind)
{
	/* Migrate legacy bind that the game merged. */
	if (aGameBind == EGameBinds::LEGACY_MoveSwimUp)
	{
		aGameBind = EGameBinds::MoveJump_SwimUp_FlyUp;
	}

	const InputBind& ib = this->Get(aGameBind);

	if (!ib.IsBound())
	{
		return;
	}

	if (ib.Type == EInputBindType::Keyboard)
	{
		int vk = MapVirtualKeyA(ib.Code, MAPVK_VSC_TO_VK_EX);
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			vk,
			GetKeyMessageLPARAM_ScanCode(ib.Code, false, false)
		);
	}
	else if (ib.Type == EInputBindType::Mouse)
	{
		/* get point for lparam */
		POINT point{};
		GetCursorPos(&point);

		switch ((EMouseButtons)ib.Code)
		{
			case EMouseButtons::LMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_LBUTTONUP,
					GetMouseMessageWPARAM(EMouseButtons::LMB, ib.Ctrl, ib.Shift, false),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::RMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_RBUTTONUP,
					GetMouseMessageWPARAM(EMouseButtons::RMB, ib.Ctrl, ib.Shift, false),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::MMB:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_MBUTTONUP,
					GetMouseMessageWPARAM(EMouseButtons::MMB, ib.Ctrl, ib.Shift, false),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::M4:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_XBUTTONUP,
					GetMouseMessageWPARAM(EMouseButtons::M4, ib.Ctrl, ib.Shift, false),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
			case EMouseButtons::M5:
			{
				this->RawInputApi->SendWndProcToGame(
					0,
					WM_XBUTTONUP,
					GetMouseMessageWPARAM(EMouseButtons::M5, ib.Ctrl, ib.Shift, false),
					MAKELPARAM(point.x, point.y)
				);
				break;
			}
		}
	}

	if (ib.Alt && !GetAsyncKeyState(VK_MENU))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_SYSKEYUP, VK_MENU, GetKeyMessageLPARAM(VK_MENU, false, true));
	}
	else if (GetAsyncKeyState(VK_MENU))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_SYSKEYDOWN, VK_MENU, GetKeyMessageLPARAM(VK_MENU, true, true));
	}

	if (ib.Ctrl && !GetAsyncKeyState(VK_CONTROL))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_KEYUP, VK_CONTROL, GetKeyMessageLPARAM(VK_CONTROL, false, false));
	}
	else if (GetAsyncKeyState(VK_CONTROL))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_KEYDOWN, VK_CONTROL, GetKeyMessageLPARAM(VK_CONTROL, true, false));
	}

	if (ib.Shift && !GetAsyncKeyState(VK_SHIFT))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_KEYUP, VK_SHIFT, GetKeyMessageLPARAM(VK_SHIFT, false, false));
	}
	else if (GetAsyncKeyState(VK_SHIFT))
	{
		this->RawInputApi->SendWndProcToGame(0, WM_KEYDOWN, VK_SHIFT, GetKeyMessageLPARAM(VK_SHIFT, true, false));
	}
}

bool CGameBindsApi::IsBound(EGameBinds aGameBind)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aGameBind);

	if (it == this->Registry.end())
	{
		return false;
	}

	return it->second.IsBound();
}

const InputBind& CGameBindsApi::Get(EGameBinds aGameBind)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aGameBind);

	if (it == this->Registry.end())
	{
		return InputBind{};
	}

	return it->second;
}

void CGameBindsApi::Set(EGameBinds aGameBind, InputBind aInputBind, bool aIsRuntimeBind)
{
	/* Remove legacy bind that the game removed. */
	if (aGameBind == EGameBinds::LEGACY_MoveSwimUp)
	{
		return;
	}

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Registry.find(aGameBind);

		if (it == this->Registry.end())
		{
			this->Registry.emplace(aGameBind, aInputBind);
		}
		else
		{
			it->second = aInputBind;
		}
	}

	if (!aIsRuntimeBind)
	{
		std::thread([this]() {
			this->EventApi->Raise("EV_INPUTBIND_UPDATED");
		}).detach();
		this->Save();
	}
}

std::unordered_map<EGameBinds, InputBind> CGameBindsApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

void CGameBindsApi::Load(std::filesystem::path aPath)
{
	this->Migrate();

	if (aPath.empty()) { aPath = Index::F_GAMEBINDS; }
	if (!std::filesystem::exists(aPath)) { return; }

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(aPath.string().c_str());

	if (!result)
	{
		this->Logger->Warning(CH_GAMEBINDS, "GameBinds.json could not be parsed. Error: %s", result.description());
		return;
	}

	pugi::xml_node root = doc.child("InputBindings");

	for (pugi::xml_node action : root.children("action"))
	{
		if (!action.attribute("id")) { continue; }

		try
		{
			EGameBinds id = (EGameBinds)strtoul(action.attribute("id").value(), nullptr, 10);
			InputBind bind{};

			if (action.attribute("device") && action.attribute("button"))
			{
				std::string device = action.attribute("device").value();
				if (device == "Keyboard")
				{
					bind.Type = EInputBindType::Keyboard;
					bind.Code = GameScanCodeToScanCode(static_cast<unsigned short>(strtoul(action.attribute("button").value(), nullptr, 10)));
				}
				else if (device == "Mouse")
				{
					bind.Type = EInputBindType::Mouse;
					std::string mousebtn = action.attribute("button").value();
					if      (mousebtn == "0") { bind.Code = (unsigned short)EMouseButtons::LMB; }
					else if (mousebtn == "2") { bind.Code = (unsigned short)EMouseButtons::RMB; }
					else if (mousebtn == "1") { bind.Code = (unsigned short)EMouseButtons::MMB; }
					else if (mousebtn == "3") { bind.Code = (unsigned short)EMouseButtons::M4; }
					else if (mousebtn == "4") { bind.Code = (unsigned short)EMouseButtons::M5; }
				}

				if (action.attribute("mod"))
				{
					int mods = strtoul(action.attribute("mod").value(), nullptr, 10);
					bind.Alt   = mods & 0b0100;
					bind.Ctrl  = mods & 0b0010;
					bind.Shift = mods & 0b0001;
				}
			}

			/* No Primary bind, try to parse secondary bind. */
			if (bind.Type == EInputBindType::None)
			{
				/* Reset before proceeding, to not have half set binds. */
				bind.Code  = 0;
				bind.Alt   = false;
				bind.Ctrl  = false;
				bind.Shift = false;

				if (action.attribute("device2") && action.attribute("button2"))
				{
					std::string device = action.attribute("device2").value();
					if (device == "Keyboard")
					{
						bind.Type = EInputBindType::Keyboard;
						bind.Code = GameScanCodeToScanCode(static_cast<unsigned short>(strtoul(action.attribute("button2").value(), nullptr, 10)));
					}
					else if (device == "Mouse")
					{
						bind.Type = EInputBindType::Mouse;
						std::string mousebtn = action.attribute("button2").value();
						if      (mousebtn == "0") { bind.Code = (unsigned short)EMouseButtons::LMB; }
						else if (mousebtn == "2") { bind.Code = (unsigned short)EMouseButtons::RMB; }
						else if (mousebtn == "1") { bind.Code = (unsigned short)EMouseButtons::MMB; }
						else if (mousebtn == "3") { bind.Code = (unsigned short)EMouseButtons::M4; }
						else if (mousebtn == "4") { bind.Code = (unsigned short)EMouseButtons::M5; }
					}

					if (action.attribute("mod2"))
					{
						int mods = strtoul(action.attribute("mod2").value(), nullptr, 10);
						bind.Alt   = mods & 0b0100;
						bind.Ctrl  = mods & 0b0010;
						bind.Shift = mods & 0b0001;
					}
				}
			}

			/* Neither primary nor secondary bind. */
			if (bind.Type == EInputBindType::None) { continue; }

			/* Store the bind. */
			this->Registry.emplace(id, bind);
		}
		catch(...) {}
	}
}

void CGameBindsApi::AddDefaultBinds()
{
	for (int i = 0; i <= 255; i++)
	{
		EGameBinds bind = static_cast<EGameBinds>(i);

		/* Remove legacy bind that the game removed. */
		if (bind == EGameBinds::LEGACY_MoveSwimUp)
		{
			continue;
		}

		if (CategoryNameFrom(bind).empty()) { continue; }

		auto it = this->Registry.find(bind);

		if (it == this->Registry.end())
		{
			this->Registry.emplace(bind, InputBind{});
		}
	}
}

void CGameBindsApi::Migrate()
{
	if (!std::filesystem::exists(Index::F_GAMEBINDS_LEGACY)) { return; }

	try
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		std::ifstream file(Index::F_GAMEBINDS_LEGACY);

		json InputBinds = json::parse(file);
		for (json binding : InputBinds)
		{
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

			EGameBinds identifier = binding["Identifier"].get<EGameBinds>();

			/* Remove legacy bind that the game removed. */
			if (identifier == EGameBinds::LEGACY_MoveSwimUp)
			{
				continue;
			}

			this->Registry.emplace(identifier, ib);
		}

		file.close();

		std::filesystem::remove(Index::F_GAMEBINDS_LEGACY); /* Delete old file. */
	}
	catch (json::parse_error& ex)
	{
		this->Logger->Warning(CH_GAMEBINDS, "GameBinds.json could not be parsed. Error: %s", ex.what());
	}

	this->Save(); /* Save as XML. */
}

void CGameBindsApi::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	pugi::xml_document doc{};
	pugi::xml_node root = doc.append_child("InputBindings");

	for (auto& it : this->Registry)
	{
		const EGameBinds& id = it.first;
		InputBind& ib = it.second;

		if (id == EGameBinds::LEGACY_MoveSwimUp) { continue; } /* Do not save legacy binds. */

		pugi::xml_node action = root.append_child("action");
		action.append_attribute("name") = this->Language->Translate(NameFrom(id).c_str()); // Purely descriptive
		action.append_attribute("id")   = std::to_string((uint32_t)id);                    // Bind ID

		switch (ib.Type)
		{
			default:
				action.append_attribute("device") = "Unknown";
				break;
			case EIBType::Keyboard:
				action.append_attribute("device") = "Keyboard";
				action.append_attribute("button") = ScanCodeToGameScanCode(ib.Code); // US/Anet Scancode
				break;
			case EIBType::Mouse:
				action.append_attribute("device") = "Mouse";
				switch ((EMouseButtons)ib.Code)
				{
					default:
					case EMouseButtons::None:
						break;
					case EMouseButtons::LMB:
						action.append_attribute("button") = "0";
						break;
					case EMouseButtons::RMB:
						action.append_attribute("button") = "2";
						break;
					case EMouseButtons::MMB:
						action.append_attribute("button") = "1";
						break;
					case EMouseButtons::M4:
						action.append_attribute("button") = "3";
						break;
					case EMouseButtons::M5:
						action.append_attribute("button") = "4";
						break;
				}
				break;
		}

		int mods = 0;
		mods += ib.Shift ? 1 : 0;
		mods += ib.Ctrl  ? 2 : 0;
		mods += ib.Alt   ? 4 : 0;

		if (mods)
		{
			action.append_attribute("mod") = std::to_string(mods); // Modifiers
		}
	}

	if (!doc.save_file(Index::F_GAMEBINDS.string().c_str(), "\t"))
	{
		this->Logger->Warning(CH_GAMEBINDS, "GameBinds.xml could not be saved.");
	}
}

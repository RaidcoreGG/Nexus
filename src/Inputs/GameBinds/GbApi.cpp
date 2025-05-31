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
#include "Index/Index.h"
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

	struct UEKey_t
	{
		unsigned DeviceType; // 0 = Unset, 1 = Mouse, 2 = Keyboard
		signed   Code; // Custom ArenaNet Scancode
		signed   Modifiers; // Bit 1 = Shfit, Bit 2 = Ctrl, Bit 3 = Alt
	};

	struct UEInputBindChanged_t
	{
		EGameBinds Identifier;
		unsigned   Index; // 0 = Primary, 1 = Secondary
		UEKey_t    Bind;
	};

	UEInputBindChanged_t* kbChange = (UEInputBindChanged_t*)aData;

	InputBind_t ib{};

	/* if key was bound (keyboard) */
	if (kbChange->Bind.DeviceType == 2)
	{
		ib.Alt = kbChange->Bind.Modifiers & 0b0100;
		ib.Ctrl = kbChange->Bind.Modifiers & 0b0010;
		ib.Shift = kbChange->Bind.Modifiers & 0b0001;

		ib.Device = EInputDevice::Keyboard;
		ib.Code = GameScanCodeToScanCode(kbChange->Bind.Code);
	}
	else if (kbChange->Bind.DeviceType == 1) /* (mouse) */
	{
		ib.Alt = kbChange->Bind.Modifiers & 0b0100;
		ib.Ctrl = kbChange->Bind.Modifiers & 0b0010;
		ib.Shift = kbChange->Bind.Modifiers & 0b0001;

		ib.Device = EInputDevice::Mouse;
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

	s_GameBindsApi->Set(kbChange->Identifier, ib, kbChange->Index == 0, true);

	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	uictx->Invalidate();
}

CGameBindsApi::CGameBindsApi(CRawInputApi* aRawInputApi, CLogApi* aLogger, CEventApi* aEventApi, CLocalization* aLocalization)
{
	assert(aRawInputApi);
	assert(aLogger);
	assert(aEventApi);

	this->RawInputApi = aRawInputApi;
	this->Logger = aLogger;
	this->EventApi = aEventApi;
	this->Language = aLocalization;

	this->Load(Index(EPath::GameBinds));
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

	const MultiInputBind_t& bind = this->Get(aGameBind);
	InputBind_t ib{};

	if (bind.Primary.IsBound())
	{
		ib = bind.Primary;
	}

	if (!ib.IsBound())
	{
		ib = bind.Secondary;
	}

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

	if (ib.Device == EInputDevice::Keyboard)
	{
		UINT vk = MapVirtualKeyA(ib.Code, MAPVK_VSC_TO_VK_EX);
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			vk,
			GetKeyMessageLPARAM_ScanCode(ib.Code, true, ib.Alt)
		);
	}
	else if (ib.Device == EInputDevice::Mouse)
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
}

void CGameBindsApi::Release(EGameBinds aGameBind)
{
	/* Migrate legacy bind that the game merged. */
	if (aGameBind == EGameBinds::LEGACY_MoveSwimUp)
	{
		aGameBind = EGameBinds::MoveJump_SwimUp_FlyUp;
	}

	const MultiInputBind_t& bind = this->Get(aGameBind);
	InputBind_t ib{};

	if (bind.Primary.IsBound())
	{
		ib = bind.Primary;
	}

	if (!ib.IsBound())
	{
		ib = bind.Secondary;
	}

	if (!ib.IsBound())
	{
		return;
	}

	if (ib.Device == EInputDevice::Keyboard)
	{
		int vk = MapVirtualKeyA(ib.Code, MAPVK_VSC_TO_VK_EX);
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			vk,
			GetKeyMessageLPARAM_ScanCode(ib.Code, false, ib.Alt)
		);
	}
	else if (ib.Device == EInputDevice::Mouse)
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

	
	this->RestoreModifiers();
}

void CGameBindsApi::RestoreModifiers()
{
	if (GetAsyncKeyState(VK_MENU))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_SYSKEYDOWN,
			VK_MENU,
			GetKeyMessageLPARAM(VK_MENU, true, true)
		);
	}
	else
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_SYSKEYUP,
			VK_MENU,
			GetKeyMessageLPARAM(VK_MENU, false, true)
		);
	}

	if (GetAsyncKeyState(VK_LCONTROL))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_LCONTROL,
			GetKeyMessageLPARAM(VK_LCONTROL, true, false)
		);
	}
	else
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_LCONTROL,
			GetKeyMessageLPARAM(VK_LCONTROL, false, false)
		);
	}
	if (GetAsyncKeyState(VK_RCONTROL))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_RCONTROL,
			GetKeyMessageLPARAM(VK_RCONTROL, true, false)
		);
	}
	else
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_RCONTROL,
			GetKeyMessageLPARAM(VK_RCONTROL, false, false)
		);
	}

	if (GetAsyncKeyState(VK_LSHIFT))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_LSHIFT,
			GetKeyMessageLPARAM(VK_LSHIFT, true, false)
		);
	}
	else
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_LSHIFT,
			GetKeyMessageLPARAM(VK_LSHIFT, false, false)
		);
	}
	if (GetAsyncKeyState(VK_RSHIFT))
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYDOWN,
			VK_RSHIFT,
			GetKeyMessageLPARAM(VK_RSHIFT, true, false)
		);
	}
	else
	{
		this->RawInputApi->SendWndProcToGame(
			0,
			WM_KEYUP,
			VK_RSHIFT,
			GetKeyMessageLPARAM(VK_RSHIFT, false, false)
		);
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

	return it->second.Primary.IsBound() || it->second.Secondary.IsBound();
}

const MultiInputBind_t& CGameBindsApi::Get(EGameBinds aGameBind)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aGameBind);

	if (it == this->Registry.end())
	{
		return MultiInputBind_t{};
	}

	return it->second;
}

void CGameBindsApi::Set(EGameBinds aGameBind, InputBind_t aInputBind, bool aIsPrimary, bool aIsRuntimeBind)
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
			MultiInputBind_t bind{};
			if (aIsPrimary)
			{
				bind.Primary = aInputBind;
			}
			else
			{
				bind.Secondary = aInputBind;
			}
			this->Registry.emplace(aGameBind, bind);
		}
		else
		{
			if (aIsPrimary)
			{
				it->second.Primary = aInputBind;
			}
			else
			{
				it->second.Secondary = aInputBind;
			}
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

std::unordered_map<EGameBinds, MultiInputBind_t> CGameBindsApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

void CGameBindsApi::Load(std::filesystem::path aPath)
{
	if (aPath.empty()) { aPath = Index(EPath::GameBinds); }
	if (!std::filesystem::exists(aPath)) { return; }

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(aPath.string().c_str());

	if (!result)
	{
		this->Logger->Warning(CH_GAMEBINDS, "GameBinds.json could not be parsed. Error: %s", result.description());
		return;
	}

	this->Registry.clear();
	this->AddDefaultBinds();

	pugi::xml_node root = doc.child("InputBindings");

	for (pugi::xml_node action : root.children("action"))
	{
		if (!action.attribute("id")) { continue; }

		try
		{
			EGameBinds id = (EGameBinds)strtoul(action.attribute("id").value(), nullptr, 10);
			MultiInputBind_t bind{};

			/* Primary Bind */
			if (action.attribute("device") && action.attribute("button"))
			{
				std::string device = action.attribute("device").value();
				if (device == "Keyboard")
				{
					bind.Primary.Device = EInputDevice::Keyboard;
					bind.Primary.Code = GameScanCodeToScanCode(static_cast<unsigned short>(strtoul(action.attribute("button").value(), nullptr, 10)));
				}
				else if (device == "Mouse")
				{
					bind.Primary.Device = EInputDevice::Mouse;
					std::string mousebtn = action.attribute("button").value();
					if      (mousebtn == "0") { bind.Primary.Code = (unsigned short)EMouseButtons::LMB; }
					else if (mousebtn == "2") { bind.Primary.Code = (unsigned short)EMouseButtons::RMB; }
					else if (mousebtn == "1") { bind.Primary.Code = (unsigned short)EMouseButtons::MMB; }
					else if (mousebtn == "3") { bind.Primary.Code = (unsigned short)EMouseButtons::M4; }
					else if (mousebtn == "4") { bind.Primary.Code = (unsigned short)EMouseButtons::M5; }
				}

				if (action.attribute("mod"))
				{
					int mods = strtoul(action.attribute("mod").value(), nullptr, 10);
					bind.Primary.Alt   = mods & 0b0100;
					bind.Primary.Ctrl  = mods & 0b0010;
					bind.Primary.Shift = mods & 0b0001;
				}
			}

			/* Secondary Bind */
			if (action.attribute("device2") && action.attribute("button2"))
			{
				std::string device = action.attribute("device2").value();
				if (device == "Keyboard")
				{
					bind.Secondary.Device = EInputDevice::Keyboard;
					bind.Secondary.Code = GameScanCodeToScanCode(static_cast<unsigned short>(strtoul(action.attribute("button2").value(), nullptr, 10)));
				}
				else if (device == "Mouse")
				{
					bind.Secondary.Device = EInputDevice::Mouse;
					std::string mousebtn = action.attribute("button2").value();
					if      (mousebtn == "0") { bind.Secondary.Code = (unsigned short)EMouseButtons::LMB; }
					else if (mousebtn == "2") { bind.Secondary.Code = (unsigned short)EMouseButtons::RMB; }
					else if (mousebtn == "1") { bind.Secondary.Code = (unsigned short)EMouseButtons::MMB; }
					else if (mousebtn == "3") { bind.Secondary.Code = (unsigned short)EMouseButtons::M4; }
					else if (mousebtn == "4") { bind.Secondary.Code = (unsigned short)EMouseButtons::M5; }
				}

				if (action.attribute("mod2"))
				{
					int mods = strtoul(action.attribute("mod2").value(), nullptr, 10);
					bind.Secondary.Alt   = mods & 0b0100;
					bind.Secondary.Ctrl  = mods & 0b0010;
					bind.Secondary.Shift = mods & 0b0001;
				}
			}

			/* Neither primary nor secondary bind. */
			if (!bind.Primary.IsBound() && !bind.Secondary.IsBound()) { continue; }

			/* Store the bind. */
			auto it = this->Registry.find(id);

			if (it == this->Registry.end())
			{
				this->Registry.emplace(id, bind);
			}
			else
			{
				it->second = bind;
			}
		}
		catch(...) {}
	}
}

void CGameBindsApi::AddDefaultBinds()
{
	// Movement
	this->Registry.emplace(EGameBinds::MoveForward, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(87) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(31) }
	});
	this->Registry.emplace(EGameBinds::MoveBackward, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(83) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(28) }
	});
	this->Registry.emplace(EGameBinds::MoveLeft, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(65) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(29) }
	});
	this->Registry.emplace(EGameBinds::MoveRight, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(68) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(30) }
	});
	this->Registry.emplace(EGameBinds::MoveTurnLeft, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(81) }
	});
	this->Registry.emplace(EGameBinds::MoveTurnRight, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(69) }
	});
	this->Registry.emplace(EGameBinds::MoveDodge, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});
	this->Registry.emplace(EGameBinds::MoveAutoRun, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(82) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(11) }
	});
	this->Registry.emplace(EGameBinds::MoveWalk, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::MoveJump_SwimUp_FlyUp, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::MoveSwimDown_FlyDown, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::MoveAboutFace, MultiInputBind_t{});

	// Skills
	this->Registry.emplace(EGameBinds::SkillWeaponSwap, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(17) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon1, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon2, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon3, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon4, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon5, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	});
	this->Registry.emplace(EGameBinds::SkillHeal, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility1, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility2, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility3, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SkillElite, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(48) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession1, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(32) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession2, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(33) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession3, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(34) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession4, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(35) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession5, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(36) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession6, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(37) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession7, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(38) }
	});
	this->Registry.emplace(EGameBinds::SkillSpecialAction, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(78) }
	});

	// Targeting
	this->Registry.emplace(EGameBinds::TargetAlert, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetCall, MultiInputBind_t{
		InputBind_t{ false, true, false, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetTake, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetCallLocal, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetTakeLocal, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetEnemyNearest, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetEnemyNext, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::TargetEnemyPrev, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::TargetAllyNearest, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAllyNext, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAllyPrev, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetLock, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetSnapGroundTarget, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetSnapGroundTargetToggle, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAutoTargetingDisable, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAutoTargetingToggle, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAllyTargetingMode, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::TargetAllyTargetingModeToggle, MultiInputBind_t{});

	// UI Binds
	this->Registry.emplace(EGameBinds::UiCommerce, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(79) }
	}); // TradingPost
	this->Registry.emplace(EGameBinds::UiContacts, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(89) }
	});
	this->Registry.emplace(EGameBinds::UiGuild, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(71) }
	});
	this->Registry.emplace(EGameBinds::UiHero, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	});
	this->Registry.emplace(EGameBinds::UiInventory, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(73) }
	});
	this->Registry.emplace(EGameBinds::UiKennel, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(75) }
	}); // Pets
	this->Registry.emplace(EGameBinds::UiLogout, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(43) }
	});
	this->Registry.emplace(EGameBinds::UiMail, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::UiOptions, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(42) }
	});
	this->Registry.emplace(EGameBinds::UiParty, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(80) }
	});
	this->Registry.emplace(EGameBinds::UiPvp, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::UiPvpBuild, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::UiScoreboard, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(66) }
	});
	this->Registry.emplace(EGameBinds::UiSeasonalObjectivesShop, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	}); // Wizard's Vault
	this->Registry.emplace(EGameBinds::UiInformation, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(7) }
	});
	this->Registry.emplace(EGameBinds::UiChatToggle, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(4) }
	});
	this->Registry.emplace(EGameBinds::UiChatCommand, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(15) }
	});
	this->Registry.emplace(EGameBinds::UiChatFocus, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(20) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(105) }
	});
	this->Registry.emplace(EGameBinds::UiChatReply, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(18) }
	});
	this->Registry.emplace(EGameBinds::UiToggle, MultiInputBind_t{
		InputBind_t{ false, true, true, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatToggle, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(4) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatCommand, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(15) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatFocus, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(20) },
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(105) }
	});

	// Camera
	this->Registry.emplace(EGameBinds::CameraFree, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::CameraZoomIn, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(27) }
	});
	this->Registry.emplace(EGameBinds::CameraZoomOut, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(26) }
	});
	this->Registry.emplace(EGameBinds::CameraReverse, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::CameraActionMode, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::CameraActionModeDisable, MultiInputBind_t{});

	// Screenshots
	this->Registry.emplace(EGameBinds::ScreenshotNormal, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(16) }
	});
	this->Registry.emplace(EGameBinds::ScreenshotStereoscopic, MultiInputBind_t{});

	// Map
	this->Registry.emplace(EGameBinds::MapToggle, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(77) }
	});
	this->Registry.emplace(EGameBinds::MapFocusPlayer, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::MapFloorDown, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(26) }
	});
	this->Registry.emplace(EGameBinds::MapFloorUp, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(27) }
	});
	this->Registry.emplace(EGameBinds::MapZoomIn, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(91) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(8) }
	});
	this->Registry.emplace(EGameBinds::MapZoomOut, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(106) },
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(7) }
	});

	// Mounts
	this->Registry.emplace(EGameBinds::SpumoniToggle, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(88) }
	});
	this->Registry.emplace(EGameBinds::SpumoniMovement, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});
	this->Registry.emplace(EGameBinds::SpumoniSecondaryMovement, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(67) }
	});
	this->Registry.emplace(EGameBinds::SpumoniMAM01, MultiInputBind_t{}); // Raptor
	this->Registry.emplace(EGameBinds::SpumoniMAM02, MultiInputBind_t{}); // Springer
	this->Registry.emplace(EGameBinds::SpumoniMAM03, MultiInputBind_t{}); // Skimmer
	this->Registry.emplace(EGameBinds::SpumoniMAM04, MultiInputBind_t{}); // Jackal
	this->Registry.emplace(EGameBinds::SpumoniMAM05, MultiInputBind_t{}); // Griffon
	this->Registry.emplace(EGameBinds::SpumoniMAM06, MultiInputBind_t{}); // RollerBeetle
	this->Registry.emplace(EGameBinds::SpumoniMAM07, MultiInputBind_t{}); // Warclaw
	this->Registry.emplace(EGameBinds::SpumoniMAM08, MultiInputBind_t{}); // Skyscale
	this->Registry.emplace(EGameBinds::SpumoniMAM09, MultiInputBind_t{}); // SiegeTurtle

	// Spectator Binds
	this->Registry.emplace(EGameBinds::SpectatorNearestFixed, MultiInputBind_t{
		InputBind_t{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::SpectatorNearestPlayer, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed1, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed2, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed3, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed4, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed5, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue1, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue2, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue3, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue4, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue5, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(48) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeCamera, MultiInputBind_t{
		InputBind_t{ false, true, true, EInputDevice::Keyboard, GameScanCodeToScanCode(70) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeCameraMode, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(69) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveForward, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(87) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveBackward, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(83) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveLeft, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(65) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveRight, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(68) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveUp, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveDown, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});

	// Squad Markers
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld1, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	}); // Arrow
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld2, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	}); // Circle
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld3, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	}); // Heart
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld4, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	}); // Square
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld5, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	}); // Star
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld6, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	}); // Swirl
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld7, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	}); // Triangle
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld8, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	}); // Cross
	this->Registry.emplace(EGameBinds::SquadMarkerClearAllWorld, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent1, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	}); // Arrow
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent2, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	}); // Circle
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent3, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	}); // Heart
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent4, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	}); // Square
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent5, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	}); // Star
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent6, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	}); // Swirl
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent7, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	}); // Triangle
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent8, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	}); // Cross
	this->Registry.emplace(EGameBinds::SquadMarkerClearAllAgent, MultiInputBind_t{
		InputBind_t{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});

	// Mastery Skills
	this->Registry.emplace(EGameBinds::MasteryAccess, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(74) }
	});
	this->Registry.emplace(EGameBinds::MasteryAccess01, MultiInputBind_t{}); // Fishing
	this->Registry.emplace(EGameBinds::MasteryAccess02, MultiInputBind_t{}); // Skiff
	this->Registry.emplace(EGameBinds::MasteryAccess03, MultiInputBind_t{}); // Jade Bot Waypoint
	this->Registry.emplace(EGameBinds::MasteryAccess04, MultiInputBind_t{}); // Rift Scan
	this->Registry.emplace(EGameBinds::MasteryAccess05, MultiInputBind_t{}); // Skyscale
	this->Registry.emplace(EGameBinds::MasteryAccess06, MultiInputBind_t{}); // Homestead Doorway

	// Miscellaneous Binds
	this->Registry.emplace(EGameBinds::MiscAoELoot, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::MiscInteract, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(70) }
	});
	this->Registry.emplace(EGameBinds::MiscShowEnemies, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(1) }
	});
	this->Registry.emplace(EGameBinds::MiscShowAllies, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(0) }
	});
	this->Registry.emplace(EGameBinds::MiscCombatStance, MultiInputBind_t{}); // Stow/Draw
	this->Registry.emplace(EGameBinds::MiscToggleLanguage, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(110) }
	});
	this->Registry.emplace(EGameBinds::MiscTogglePetCombat, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::MiscToggleFullScreen, MultiInputBind_t{
		InputBind_t{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(20) }
	});
	this->Registry.emplace(EGameBinds::MiscToggleDecorationMode, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(76) }
	}); // Decoration Mode

	// Toys/Novelties
	this->Registry.emplace(EGameBinds::ToyUseDefault, MultiInputBind_t{
		InputBind_t{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(85) }
	});
	this->Registry.emplace(EGameBinds::ToyUseSlot1, MultiInputBind_t{}); // Chair
	this->Registry.emplace(EGameBinds::ToyUseSlot2, MultiInputBind_t{}); // Instrument
	this->Registry.emplace(EGameBinds::ToyUseSlot3, MultiInputBind_t{}); // Held Item
	this->Registry.emplace(EGameBinds::ToyUseSlot4, MultiInputBind_t{}); // Toy
	this->Registry.emplace(EGameBinds::ToyUseSlot5, MultiInputBind_t{}); // Tonic
	//ToyUseSlot6 unused

	// Build Templates
	this->Registry.emplace(EGameBinds::Loadout1, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout2, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout3, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout4, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout5, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout6, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout7, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout8, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::Loadout9, MultiInputBind_t{});

	// Equipment Templates
	this->Registry.emplace(EGameBinds::GearLoadout1, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout2, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout3, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout4, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout5, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout6, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout7, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout8, MultiInputBind_t{});
	this->Registry.emplace(EGameBinds::GearLoadout9, MultiInputBind_t{});
}

void CGameBindsApi::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	pugi::xml_document doc{};
	pugi::xml_node root = doc.append_child("InputBindings");

	for (auto& it : this->Registry)
	{
		const EGameBinds& id = it.first;
		MultiInputBind_t& ib = it.second;

		if (id == EGameBinds::LEGACY_MoveSwimUp) { continue; } /* Do not save legacy binds. */
		if (ib.Primary.Device == EInputDevice::None && ib.Secondary.Device == EInputDevice::None)
		{
			/* Do not save unset binds. */
			continue;
		}

		pugi::xml_node action = root.append_child("action");
		action.append_attribute("name") = this->Language->Translate(NameFrom(id).c_str()); // Purely descriptive
		action.append_attribute("id")   = std::to_string((uint32_t)id);                    // Bind ID

		if (ib.Primary.Device != EInputDevice::None)
		{
			switch (ib.Primary.Device)
			{
				default:
					action.append_attribute("device") = "Unknown";
					break;
				case EInputDevice::Keyboard:
					action.append_attribute("device") = "Keyboard";
					action.append_attribute("button") = ScanCodeToGameScanCode(ib.Primary.Code); // US/Anet Scancode
					break;
				case EInputDevice::Mouse:
					action.append_attribute("device") = "Mouse";
					switch ((EMouseButtons)ib.Primary.Code)
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
			mods += ib.Primary.Shift ? 1 : 0;
			mods += ib.Primary.Ctrl  ? 2 : 0;
			mods += ib.Primary.Alt   ? 4 : 0;

			if (mods)
			{
				action.append_attribute("mod") = std::to_string(mods); // Modifiers
			}
		}

		if (ib.Secondary.Device != EInputDevice::None)
		{
			switch (ib.Secondary.Device)
			{
				default:
					action.append_attribute("device2") = "Unknown";
					break;
				case EInputDevice::Keyboard:
					action.append_attribute("device2") = "Keyboard";
					action.append_attribute("button2") = ScanCodeToGameScanCode(ib.Secondary.Code); // US/Anet Scancode
					break;
				case EInputDevice::Mouse:
					action.append_attribute("device2") = "Mouse";
					switch ((EMouseButtons)ib.Secondary.Code)
					{
						default:
						case EMouseButtons::None:
							break;
						case EMouseButtons::LMB:
							action.append_attribute("button2") = "0";
							break;
						case EMouseButtons::RMB:
							action.append_attribute("button2") = "2";
							break;
						case EMouseButtons::MMB:
							action.append_attribute("button2") = "1";
							break;
						case EMouseButtons::M4:
							action.append_attribute("button2") = "3";
							break;
						case EMouseButtons::M5:
							action.append_attribute("button2") = "4";
							break;
					}
					break;
			}

			int mods2 = 0;
			mods2 += ib.Secondary.Shift ? 1 : 0;
			mods2 += ib.Secondary.Ctrl  ? 2 : 0;
			mods2 += ib.Secondary.Alt   ? 4 : 0;

			if (mods2)
			{
				action.append_attribute("mod2") = std::to_string(mods2); // Modifiers
			}
		}
	}

	if (!doc.save_file(Index(EPath::GameBinds).string().c_str(), "\t"))
	{
		this->Logger->Warning(CH_GAMEBINDS, "GameBinds.xml could not be saved.");
	}
}

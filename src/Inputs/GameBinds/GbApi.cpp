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

	InputBind ib{};

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

CGameBindsApi::CGameBindsApi(CRawInputApi* aRawInputApi, CLogHandler* aLogger, CEventApi* aEventApi, CLocalization* aLocalization)
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

	const MultiInputBind& bind = this->Get(aGameBind);
	InputBind ib{};

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

	const MultiInputBind& bind = this->Get(aGameBind);
	InputBind ib{};

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

const MultiInputBind& CGameBindsApi::Get(EGameBinds aGameBind)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aGameBind);

	if (it == this->Registry.end())
	{
		return MultiInputBind{};
	}

	return it->second;
}

void CGameBindsApi::Set(EGameBinds aGameBind, InputBind aInputBind, bool aIsPrimary, bool aIsRuntimeBind)
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
			MultiInputBind bind{};
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

std::unordered_map<EGameBinds, MultiInputBind> CGameBindsApi::GetRegistry() const
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
			MultiInputBind bind{};

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
	this->Registry.emplace(EGameBinds::MoveForward, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(87) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(31) }
	});
	this->Registry.emplace(EGameBinds::MoveBackward, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(83) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(28) }
	});
	this->Registry.emplace(EGameBinds::MoveLeft, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(65) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(29) }
	});
	this->Registry.emplace(EGameBinds::MoveRight, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(68) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(30) }
	});
	this->Registry.emplace(EGameBinds::MoveTurnLeft, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(81) }
	});
	this->Registry.emplace(EGameBinds::MoveTurnRight, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(69) }
	});
	this->Registry.emplace(EGameBinds::MoveDodge, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});
	this->Registry.emplace(EGameBinds::MoveAutoRun, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(82) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(11) }
	});
	this->Registry.emplace(EGameBinds::MoveWalk, MultiInputBind{});
	this->Registry.emplace(EGameBinds::MoveJump_SwimUp_FlyUp, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::MoveSwimDown_FlyDown, MultiInputBind{});
	this->Registry.emplace(EGameBinds::MoveAboutFace, MultiInputBind{});

	// Skills
	this->Registry.emplace(EGameBinds::SkillWeaponSwap, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(17) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon1, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon2, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon3, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon4, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	});
	this->Registry.emplace(EGameBinds::SkillWeapon5, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	});
	this->Registry.emplace(EGameBinds::SkillHeal, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility1, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility2, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	});
	this->Registry.emplace(EGameBinds::SkillUtility3, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SkillElite, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(48) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession1, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(32) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession2, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(33) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession3, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(34) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession4, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(35) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession5, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(36) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession6, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(37) }
	});
	this->Registry.emplace(EGameBinds::SkillProfession7, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(38) }
	});
	this->Registry.emplace(EGameBinds::SkillSpecialAction, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(78) }
	});

	// Targeting
	this->Registry.emplace(EGameBinds::TargetAlert, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetCall, MultiInputBind{
		InputBind{ false, true, false, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetTake, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(84) }
	});
	this->Registry.emplace(EGameBinds::TargetCallLocal, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetTakeLocal, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetEnemyNearest, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetEnemyNext, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::TargetEnemyPrev, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::TargetAllyNearest, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAllyNext, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAllyPrev, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetLock, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetSnapGroundTarget, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetSnapGroundTargetToggle, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAutoTargetingDisable, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAutoTargetingToggle, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAllyTargetingMode, MultiInputBind{});
	this->Registry.emplace(EGameBinds::TargetAllyTargetingModeToggle, MultiInputBind{});

	// UI Binds
	this->Registry.emplace(EGameBinds::UiCommerce, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(79) }
	}); // TradingPost
	this->Registry.emplace(EGameBinds::UiContacts, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(89) }
	});
	this->Registry.emplace(EGameBinds::UiGuild, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(71) }
	});
	this->Registry.emplace(EGameBinds::UiHero, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	});
	this->Registry.emplace(EGameBinds::UiInventory, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(73) }
	});
	this->Registry.emplace(EGameBinds::UiKennel, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(75) }
	}); // Pets
	this->Registry.emplace(EGameBinds::UiLogout, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(43) }
	});
	this->Registry.emplace(EGameBinds::UiMail, MultiInputBind{});
	this->Registry.emplace(EGameBinds::UiOptions, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(42) }
	});
	this->Registry.emplace(EGameBinds::UiParty, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(80) }
	});
	this->Registry.emplace(EGameBinds::UiPvp, MultiInputBind{});
	this->Registry.emplace(EGameBinds::UiPvpBuild, MultiInputBind{});
	this->Registry.emplace(EGameBinds::UiScoreboard, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(66) }
	});
	this->Registry.emplace(EGameBinds::UiSeasonalObjectivesShop, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	}); // Wizard's Vault
	this->Registry.emplace(EGameBinds::UiInformation, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(7) }
	});
	this->Registry.emplace(EGameBinds::UiChatToggle, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(4) }
	});
	this->Registry.emplace(EGameBinds::UiChatCommand, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(15) }
	});
	this->Registry.emplace(EGameBinds::UiChatFocus, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(20) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(105) }
	});
	this->Registry.emplace(EGameBinds::UiChatReply, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(18) }
	});
	this->Registry.emplace(EGameBinds::UiToggle, MultiInputBind{
		InputBind{ false, true, true, EInputDevice::Keyboard, GameScanCodeToScanCode(72) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatToggle, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(4) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatCommand, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(15) }
	});
	this->Registry.emplace(EGameBinds::UiSquadBroadcastChatFocus, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(20) },
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(105) }
	});

	// Camera
	this->Registry.emplace(EGameBinds::CameraFree, MultiInputBind{});
	this->Registry.emplace(EGameBinds::CameraZoomIn, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(27) }
	});
	this->Registry.emplace(EGameBinds::CameraZoomOut, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(26) }
	});
	this->Registry.emplace(EGameBinds::CameraReverse, MultiInputBind{});
	this->Registry.emplace(EGameBinds::CameraActionMode, MultiInputBind{});
	this->Registry.emplace(EGameBinds::CameraActionModeDisable, MultiInputBind{});

	// Screenshots
	this->Registry.emplace(EGameBinds::ScreenshotNormal, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(16) }
	});
	this->Registry.emplace(EGameBinds::ScreenshotStereoscopic, MultiInputBind{});

	// Map
	this->Registry.emplace(EGameBinds::MapToggle, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(77) }
	});
	this->Registry.emplace(EGameBinds::MapFocusPlayer, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::MapFloorDown, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(26) }
	});
	this->Registry.emplace(EGameBinds::MapFloorUp, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(27) }
	});
	this->Registry.emplace(EGameBinds::MapZoomIn, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(91) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(8) }
	});
	this->Registry.emplace(EGameBinds::MapZoomOut, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(106) },
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(7) }
	});

	// Mounts
	this->Registry.emplace(EGameBinds::SpumoniToggle, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(88) }
	});
	this->Registry.emplace(EGameBinds::SpumoniMovement, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});
	this->Registry.emplace(EGameBinds::SpumoniSecondaryMovement, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(67) }
	});
	this->Registry.emplace(EGameBinds::SpumoniMAM01, MultiInputBind{}); // Raptor
	this->Registry.emplace(EGameBinds::SpumoniMAM02, MultiInputBind{}); // Springer
	this->Registry.emplace(EGameBinds::SpumoniMAM03, MultiInputBind{}); // Skimmer
	this->Registry.emplace(EGameBinds::SpumoniMAM04, MultiInputBind{}); // Jackal
	this->Registry.emplace(EGameBinds::SpumoniMAM05, MultiInputBind{}); // Griffon
	this->Registry.emplace(EGameBinds::SpumoniMAM06, MultiInputBind{}); // RollerBeetle
	this->Registry.emplace(EGameBinds::SpumoniMAM07, MultiInputBind{}); // Warclaw
	this->Registry.emplace(EGameBinds::SpumoniMAM08, MultiInputBind{}); // Skyscale
	this->Registry.emplace(EGameBinds::SpumoniMAM09, MultiInputBind{}); // SiegeTurtle

	// Spectator Binds
	this->Registry.emplace(EGameBinds::SpectatorNearestFixed, MultiInputBind{
		InputBind{ false, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::SpectatorNearestPlayer, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(22) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed1, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed2, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed3, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed4, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerRed5, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue1, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue2, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue3, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue4, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SpectatorPlayerBlue5, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(48) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeCamera, MultiInputBind{
		InputBind{ false, true, true, EInputDevice::Keyboard, GameScanCodeToScanCode(70) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeCameraMode, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(69) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveForward, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(87) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveBackward, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(83) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveLeft, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(65) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveRight, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(68) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveUp, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(21) }
	});
	this->Registry.emplace(EGameBinds::SpectatorFreeMoveDown, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(86) }
	});

	// Squad Markers
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld1, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	}); // Arrow
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld2, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	}); // Circle
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld3, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	}); // Heart
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld4, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	}); // Square
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld5, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	}); // Star
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld6, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	}); // Swirl
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld7, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	}); // Triangle
	this->Registry.emplace(EGameBinds::SquadMarkerPlaceWorld8, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	}); // Cross
	this->Registry.emplace(EGameBinds::SquadMarkerClearAllWorld, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent1, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(49) }
	}); // Arrow
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent2, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(50) }
	}); // Circle
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent3, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(51) }
	}); // Heart
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent4, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(52) }
	}); // Square
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent5, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(53) }
	}); // Star
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent6, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(54) }
	}); // Swirl
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent7, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(55) }
	}); // Triangle
	this->Registry.emplace(EGameBinds::SquadMarkerSetAgent8, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(56) }
	}); // Cross
	this->Registry.emplace(EGameBinds::SquadMarkerClearAllAgent, MultiInputBind{
		InputBind{ true, false, true, EInputDevice::Keyboard, GameScanCodeToScanCode(57) }
	});

	// Mastery Skills
	this->Registry.emplace(EGameBinds::MasteryAccess, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(74) }
	});
	this->Registry.emplace(EGameBinds::MasteryAccess01, MultiInputBind{}); // Fishing
	this->Registry.emplace(EGameBinds::MasteryAccess02, MultiInputBind{}); // Skiff
	this->Registry.emplace(EGameBinds::MasteryAccess03, MultiInputBind{}); // Jade Bot Waypoint
	this->Registry.emplace(EGameBinds::MasteryAccess04, MultiInputBind{}); // Rift Scan
	this->Registry.emplace(EGameBinds::MasteryAccess05, MultiInputBind{}); // Skyscale
	this->Registry.emplace(EGameBinds::MasteryAccess06, MultiInputBind{}); // Homestead Doorway

	// Miscellaneous Binds
	this->Registry.emplace(EGameBinds::MiscAoELoot, MultiInputBind{});
	this->Registry.emplace(EGameBinds::MiscInteract, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(70) }
	});
	this->Registry.emplace(EGameBinds::MiscShowEnemies, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(1) }
	});
	this->Registry.emplace(EGameBinds::MiscShowAllies, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(0) }
	});
	this->Registry.emplace(EGameBinds::MiscCombatStance, MultiInputBind{}); // Stow/Draw
	this->Registry.emplace(EGameBinds::MiscToggleLanguage, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(110) }
	});
	this->Registry.emplace(EGameBinds::MiscTogglePetCombat, MultiInputBind{});
	this->Registry.emplace(EGameBinds::MiscToggleFullScreen, MultiInputBind{
		InputBind{ true, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(20) }
	});
	this->Registry.emplace(EGameBinds::MiscToggleDecorationMode, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(76) }
	}); // Decoration Mode

	// Toys/Novelties
	this->Registry.emplace(EGameBinds::ToyUseDefault, MultiInputBind{
		InputBind{ false, false, false, EInputDevice::Keyboard, GameScanCodeToScanCode(85) }
	});
	this->Registry.emplace(EGameBinds::ToyUseSlot1, MultiInputBind{}); // Chair
	this->Registry.emplace(EGameBinds::ToyUseSlot2, MultiInputBind{}); // Instrument
	this->Registry.emplace(EGameBinds::ToyUseSlot3, MultiInputBind{}); // Held Item
	this->Registry.emplace(EGameBinds::ToyUseSlot4, MultiInputBind{}); // Toy
	this->Registry.emplace(EGameBinds::ToyUseSlot5, MultiInputBind{}); // Tonic
	//ToyUseSlot6 unused

	// Build Templates
	this->Registry.emplace(EGameBinds::Loadout1, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout2, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout3, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout4, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout5, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout6, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout7, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout8, MultiInputBind{});
	this->Registry.emplace(EGameBinds::Loadout9, MultiInputBind{});

	// Equipment Templates
	this->Registry.emplace(EGameBinds::GearLoadout1, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout2, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout3, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout4, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout5, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout6, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout7, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout8, MultiInputBind{});
	this->Registry.emplace(EGameBinds::GearLoadout9, MultiInputBind{});
}

void CGameBindsApi::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	pugi::xml_document doc{};
	pugi::xml_node root = doc.append_child("InputBindings");

	for (auto& it : this->Registry)
	{
		const EGameBinds& id = it.first;
		MultiInputBind& ib = it.second;

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

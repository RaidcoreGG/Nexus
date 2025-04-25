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

CGameBindsApi::CGameBindsApi(CRawInputApi* aRawInputApi, CLogHandler* aLogger, CEventApi* aEventApi)
{
	assert(aRawInputApi);
	assert(aLogger);
	assert(aEventApi);

	this->RawInputApi = aRawInputApi;
	this->Logger = aLogger;
	this->EventApi = aEventApi;

	this->Load();
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

void CGameBindsApi::Import(std::filesystem::path aPath)
{
	// TODO: https://github.com/RaidcoreGG/Nexus/issues/63
	return;
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

		if (aIsRuntimeBind)
		{
			this->IsReceivingRuntimeBinds = true;
		}
		else
		{
			std::thread([this]() {
				this->EventApi->Raise("EV_INPUTBIND_UPDATED");
			}).detach();
		}
	}

	this->Save();
}

std::unordered_map<EGameBinds, InputBind> CGameBindsApi::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
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

void CGameBindsApi::Load()
{
	if (!std::filesystem::exists(Index::F_GAMEBINDS)) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	try
	{
		std::ifstream file(Index::F_GAMEBINDS);

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
	}
	catch (json::parse_error& ex)
	{
		Logger->Warning(CH_GAMEBINDS, "InputBinds.json could not be parsed. Error: %s", ex.what());
	}
}

void CGameBindsApi::Save()
{
	if (this->IsReceivingRuntimeBinds) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	json InputBinds = json::array();

	for (auto& it : this->Registry)
	{
		EGameBinds id = it.first;
		InputBind ib = it.second;

		/* Remove legacy bind that the game removed. */
		if (id == EGameBinds::LEGACY_MoveSwimUp)
		{
			continue;
		}

		if (!ib.IsBound()) { continue; }

		json binding =
		{
			{"Identifier", id},
			{"Alt",        ib.Alt},
			{"Ctrl",       ib.Ctrl},
			{"Shift",      ib.Shift},
			{"Type",       ib.Type},
			{"Code",       ib.Code}
		};

		/* override type */
		if (!ib.Code)
		{
			ib.Type = EInputBindType::None;
		}
		InputBinds.push_back(binding);
	}

	std::ofstream file(Index::F_GAMEBINDS);

	file << InputBinds.dump(1, '\t') << std::endl;

	file.close();
}

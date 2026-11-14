///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtRouter.cpp
/// Description  :  Routes events raised by other addons to engine notifications.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "EvtRouter.h"

#include <cassert>
#include <iterator>

#include "Host/GW2/Inputs/GameBinds/GbConst.h"
#include "Util/Inputs.h"

/* The event callbacks carry no context. */
static CEventRouter* s_EventRouter{};

namespace UnofficialExtras
{
	struct Key_t
	{
		unsigned DeviceType; /* 0 = Unset, 1 = Mouse, 2 = Keyboard */
		signed   Code;       /* ArenaNet scancode */
		signed   Modifiers;  /* Bit 1 = Shift, Bit 2 = Ctrl, Bit 3 = Alt */
	};

	struct KeybindChanged_t
	{
		EGameBinds Identifier;
		unsigned   Index;    /* 0 = Primary, 1 = Secondary */
		Key_t      Bind;
	};

	static InputBind_t ToInputBind(const Key_t& aKey)
	{
		InputBind_t ib{};

		if (aKey.DeviceType != 1 && aKey.DeviceType != 2) { return ib; }

		ib.Alt   = aKey.Modifiers & 0b0100;
		ib.Ctrl  = aKey.Modifiers & 0b0010;
		ib.Shift = aKey.Modifiers & 0b0001;

		if (aKey.DeviceType == 2)
		{
			ib.Device = EInputDevice::Keyboard;
			ib.Code   = GameScanCodeToScanCode(aKey.Code);
			return ib;
		}

		ib.Device = EInputDevice::Mouse;

		switch (aKey.Code)
		{
			case 0: { ib.Code = (unsigned short)EMouseButtons::LMB; break; }
			case 1: { ib.Code = (unsigned short)EMouseButtons::MMB; break; }
			case 2: { ib.Code = (unsigned short)EMouseButtons::RMB; break; }
			case 3: { ib.Code = (unsigned short)EMouseButtons::M4;  break; }
			case 4: { ib.Code = (unsigned short)EMouseButtons::M5;  break; }
		}

		return ib;
	}
}

CEventRouter::CEventRouter(CEventApi* aEventApi) : ILogChannel("EventRouter")
{
	assert(aEventApi);
	assert(!s_EventRouter);

	this->EventApi = aEventApi;
	s_EventRouter = this;

	this->EventApi->Subscribe(EV_UE_KB_CH,       CEventRouter::OnUEKeybindChanged);
	this->EventApi->Subscribe(EV_UE_LANGUAGE_CH, CEventRouter::OnUELanguageChanged);
}

CEventRouter::~CEventRouter()
{
	this->EventApi->Unsubscribe(EV_UE_LANGUAGE_CH, CEventRouter::OnUELanguageChanged);
	this->EventApi->Unsubscribe(EV_UE_KB_CH,       CEventRouter::OnUEKeybindChanged);

	s_EventRouter = nullptr;
}

void CEventRouter::OnUEKeybindChanged(void* aEventData)
{
	if (!s_EventRouter || !aEventData) { return; }

	const UnofficialExtras::KeybindChanged_t* change = static_cast<const UnofficialExtras::KeybindChanged_t*>(aEventData);

	s_EventRouter->NotifyGameBindChanged(change->Identifier, UnofficialExtras::ToInputBind(change->Bind), change->Index == 0);
}

void CEventRouter::OnUELanguageChanged(void* aEventData)
{
	if (!s_EventRouter || !aEventData) { return; }

	static constexpr const char* s_Languages[] = { "en", "kr", "fr", "de", "es", "cn" };

	uint32_t language = *static_cast<const uint32_t*>(aEventData);

	if (language >= std::size(s_Languages))
	{
		s_EventRouter->LogDebug("Unknown game language %u from Unofficial Extras.", language);
		return;
	}

	s_EventRouter->NotifyGameLanguage(s_Languages[language]);
}

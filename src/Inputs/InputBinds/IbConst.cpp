///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbConst.cpp
/// Description  :  Constant data for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbConst.h"

#include <windows.h>
#include <unordered_map>

#include "Consts.h"
#include "Util/Inputs.h"
#include "Util/Strings.h"

InputBind IBFromString(std::string aInputBind)
{
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
		ib.Device = EInputDevice::Mouse;
		ib.Code = (unsigned short)EMouseButtons::LMB;
	}
	else if (aInputBind == "RMB")
	{
		ib.Device = EInputDevice::Mouse;
		ib.Code = (unsigned short)EMouseButtons::RMB;
	}
	else if (aInputBind == "MMB")
	{
		ib.Device = EInputDevice::Mouse;
		ib.Code = (unsigned short)EMouseButtons::MMB;
	}
	else if (aInputBind == "M4")
	{
		ib.Device = EInputDevice::Mouse;
		ib.Code = (unsigned short)EMouseButtons::M4;
	}
	else if (aInputBind == "M5")
	{
		ib.Device = EInputDevice::Mouse;
		ib.Code = (unsigned short)EMouseButtons::M5;
	}
	else
	{
		ib.Device = EInputDevice::Keyboard;

		static std::unordered_map<std::string, unsigned short> s_ScancodeLUT;
		static bool s_IsLUTBuilt = [] {
			for (long long i = 0; i <= 255; i++)
			{
				/* create key msg lparam */
				KeyLParam key{};
				key.ScanCode = i;

				/* get the scancode (current i) */
				{
					char buff[256]{};
					GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, sizeof(buff));
					s_ScancodeLUT.emplace(buff, key.GetScanCode());
				}

				key.ExtendedFlag = 1;

				/* get the scancode again, but this time with the extended flag */
				{
					char buff[256]{};
					GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, sizeof(buff));
					s_ScancodeLUT.emplace(buff, key.GetScanCode());
				}
			};
			return true;
		}();

		auto it = s_ScancodeLUT.find(aInputBind);

		if (it != s_ScancodeLUT.end())
		{
			ib.Code = it->second;
		}
	}

	return ib;
}

std::string IBToString(const InputBind& aInputBind, bool aPadded)
{
	if (aInputBind.Device == EInputDevice::None)                          { return NULLSTR; }
	if (aInputBind.Device == EInputDevice::Mouse && aInputBind.Code == 0) { return NULLSTR; }
	if (aInputBind.Device == EInputDevice::Keyboard
		&& !aInputBind.Code
		&& !aInputBind.Alt
		&& !aInputBind.Ctrl
		&& !aInputBind.Shift)
	{
		return NULLSTR;
	}

	char buff[100]{};
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

	if (aInputBind.Device == EInputDevice::Keyboard)
	{
		GetKeyNameTextA(static_cast<LONG>(GetKeyMessageLPARAM_ScanCode(aInputBind.Code, false, false)), buff, 100);
		str.append(buff);
	}
	else if (aInputBind.Device == EInputDevice::Mouse)
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

	str = String::ToUpper(str);

	return String::ConvertMBToUTF8(str);
}

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcFuncDefs.h
/// Description  :  Function definitions for ArcDPS API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <windows.h>

#include "ArcEnum.h"
#include "ArcExtensionDef.h"

///----------------------------------------------------------------------------------------------------
/// ArcDPS Namespace
///----------------------------------------------------------------------------------------------------
namespace ArcDPS
{
	///----------------------------------------------------------------------------------------------------
	/// UiFlags_t Struct
	///----------------------------------------------------------------------------------------------------
	struct UiFlags_t
	{
		uint64_t IsHidden     : 1;
		uint64_t AlwaysDraw   : 1;
		uint64_t MoveLocked   : 1;
		uint64_t ClickLocked  : 1;
		uint64_t CloseWithEsc : 1;
	};

	///----------------------------------------------------------------------------------------------------
	/// Modifiers_t Struct
	///----------------------------------------------------------------------------------------------------
	struct Modifiers_t
	{
		uint16_t Mod1;
		uint16_t Mod2;
		uint16_t ModMulti;
		uint16_t _PAD;
	};

	/* e0 */
	typedef wchar_t*      (*PFN_GETARCDPSINIPATH)();

	/* e3 */
	typedef void          (*PFN_LOGFILE)         (char* aMessage);

	/* e6 */
	typedef UiFlags_t     (*PFN_GETUIFLAGS)      ();

	/* e7 */
	typedef Modifiers_t   (*PFN_GETMODIFIERS)    ();

	/* e8 */
	typedef void          (*PFN_LOGWINDOW)       (char* aMessage);

	/* addextension2 */
	typedef EAddExtResult (*PFN_ADDEXTENSION2)   (HMODULE aModule);

	/*freeextension2 */
	typedef HMODULE       (*PFN_FREEEXTENSION2)  (uint32_t aSignature);

	typedef void          (*PFN_RECEIVEEXTENSION)(ExtensionDefRaw_t* aExtensionDef);

	/* listextension */
	typedef void          (*PFN_LISTEXTENSION)   (PFN_RECEIVEEXTENSION aCallback);
}

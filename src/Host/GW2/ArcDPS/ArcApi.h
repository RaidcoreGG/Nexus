///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcApi.h
/// Description  :  API to call native ArcDPS functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <filesystem>
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

///----------------------------------------------------------------------------------------------------
/// CArcApi Class
///----------------------------------------------------------------------------------------------------
class CArcApi
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CArcApi();

	///----------------------------------------------------------------------------------------------------
	/// IsInitialized:
	/// 	Returns true, if ArcDPS is loaded.
	///----------------------------------------------------------------------------------------------------
	bool IsInitialized();

	///----------------------------------------------------------------------------------------------------
	/// Initialize:
	/// 	Initializes the ArcDPS API with the provided module.
	/// 	Throws if module not ArcDPS.
	///----------------------------------------------------------------------------------------------------
	void Initialize(HMODULE aArcdpsModule);

	///----------------------------------------------------------------------------------------------------
	/// TryDetect:
	/// 	Attempts to detect if ArcDPS is loaded in the current process using known paths.
	/// 	Returns true if ArcDPS is loaded and initializes the API.
	///----------------------------------------------------------------------------------------------------
	bool TryDetect();

	///----------------------------------------------------------------------------------------------------
	/// GetArcIniPath:
	/// 	Returns the path of ArcDPS' .ini file.
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetArcIniPath();

	///----------------------------------------------------------------------------------------------------
	/// LogToFile:
	/// 	Logs to ArcDPS' log file.
	///----------------------------------------------------------------------------------------------------
	void LogToFile(char* aMessage);

	///----------------------------------------------------------------------------------------------------
	/// LogToWindow:
	/// 	Logs to ArcDPS' log window.
	///----------------------------------------------------------------------------------------------------
	void LogToWindow(char* aMessage);

	///----------------------------------------------------------------------------------------------------
	/// GetUIFlags:
	/// 	Returns ArcDPS' current UI settings.
	///----------------------------------------------------------------------------------------------------
	ArcDPS::UiFlags_t GetUIFlags();

	///----------------------------------------------------------------------------------------------------
	/// GetModifiers:
	/// 	Returns ArcDPS' current modifier keys.
	///----------------------------------------------------------------------------------------------------
	ArcDPS::Modifiers_t GetModifiers();

	///----------------------------------------------------------------------------------------------------
	/// AddExtension:
	/// 	Adds an extension to ArcDPS.
	///----------------------------------------------------------------------------------------------------
	ArcDPS::EAddExtResult AddExtension(HMODULE aModule);

	///----------------------------------------------------------------------------------------------------
	/// FreeExtension:
	/// 	Adds an extension to ArcDPS.
	///----------------------------------------------------------------------------------------------------
	void FreeExtension(uint32_t aSignature);

	///----------------------------------------------------------------------------------------------------
	/// PollExtensions:
	/// 	Updates the information which extensions are currently loaded.
	///----------------------------------------------------------------------------------------------------
	void PollExtensions();

	///----------------------------------------------------------------------------------------------------
	/// ReceiveExtension:
	/// 	Extension receiver callback function for ListExtensions API call.
	///----------------------------------------------------------------------------------------------------
	void ReceiveExtension(ArcDPS::ExtensionDefRaw_t* aExtensionDef);

	private:
	struct FuncTable_t
	{
		ArcDPS::PFN_GETARCDPSINIPATH GetArcIniPath;
		ArcDPS::PFN_LOGFILE          LogToFile;
		ArcDPS::PFN_LOGWINDOW        LogToWindow;
		ArcDPS::PFN_GETUIFLAGS       GetUIFlags;
		ArcDPS::PFN_GETMODIFIERS     GetModifiers;
		ArcDPS::PFN_ADDEXTENSION2    AddExtension;
		ArcDPS::PFN_FREEEXTENSION2   FreeExtension;
		ArcDPS::PFN_LISTEXTENSION    ListExtensions;
	};

	HMODULE     Module    = nullptr;
	FuncTable_t Functions = {};

	///----------------------------------------------------------------------------------------------------
	/// BuildFunctionTable:
	/// 	Builds the function table used to interface with ArcDPS.
	/// 	Returns true, if the function table was built.
	/// 	Returns false, if functions were missing from the provided module.
	///----------------------------------------------------------------------------------------------------
	bool BuildFunctionTable(HMODULE aArcdpsModule);
};

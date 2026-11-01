///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcApi.h
/// Description  :  API to call native ArcDPS functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <windows.h>

#include "ArcFuncDefs.h"

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

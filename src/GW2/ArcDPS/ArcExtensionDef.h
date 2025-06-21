///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ArcExtensionDef.h
/// Description  :  Extension definition for ArcDPS API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ARCEXTENSIONDEF_H
#define ARCEXTENSIONDEF_H

#include <cstdint>
#include <string>

///----------------------------------------------------------------------------------------------------
/// ArcDPS Namespace
///----------------------------------------------------------------------------------------------------
namespace ArcDPS
{
	///----------------------------------------------------------------------------------------------------
	/// ExtensionDefRaw_t Struct
	///----------------------------------------------------------------------------------------------------
	struct ExtensionDefRaw_t
	{
		uint64_t    Size;
		uint32_t    Signature;
		uint32_t    ImGuiVersion;
		const char* Name;
		const char* Build;
		void*       WndProc;
		void*       Combat;
		void*       Render;
		void*       RenderOptionsTab;
		void*       CombatLocal;
		void*       WndProcFiltered;
		void*       RenderOptionsWindow;
	};

	///----------------------------------------------------------------------------------------------------
	/// ExtensionDef_t Struct
	///----------------------------------------------------------------------------------------------------
	struct ExtensionDef_t
	{
		uint32_t    Signature;
		std::string Name;
		std::string Build;
	};
}

#endif

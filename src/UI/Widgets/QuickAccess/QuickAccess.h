///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QuickAccess.h
/// Description  :  Contains the logic for the Quick Access HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QUICKACCESS_H
#define QUICKACCESS_H

#include <map>
#include <mutex>
#include <string>

#include "imgui/imgui.h"

#include "QaEnum.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Loader/NexusLinkData.h"
#include "Engine/DataLink/DlApi.h"
#include "UI/Services/Localization/LoclApi.h"
#include "Engine/Logging/LogApi.h"
#include "GW2/Mumble/MblReader.h"
#include "Engine/Textures/TxLoader.h"
#include "QaShortcut.h"
#include "UI/Controls/CtlWindow.h"
#include "UI/FuncDefs.h"

constexpr const char* CH_QUICKACCESS = "Quick Access";
constexpr const char* QA_MENU        = "0_QA_MENU";
constexpr const char* QA_ARCDPS      = "QA_ARCDPS";

///----------------------------------------------------------------------------------------------------
/// CQuickAccess Class
///----------------------------------------------------------------------------------------------------
class CQuickAccess : public virtual IWindow
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnAddonLoaded:
	/// 	Rechecks invalid shortcuts on addon load.
	///----------------------------------------------------------------------------------------------------
	static void OnAddonLoaded(void* aEventData);

	public:
	bool                               VerticalLayout     = false;
	bool                               ShowArcDPSShortcut = true;
	EQaVisibility                      Visibility         = EQaVisibility::AlwaysShow;
	EQaPosition                        Location           = EQaPosition::Extend;
	ImVec2                             Offset             = ImVec2(0, 0);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CQuickAccess(CDataLinkApi* aDataLink, CLogApi* aLogger, CInputBindApi* aInputBindApi, CTextureLoader* aTextureService, CLocalization* aLocalization, CEventApi* aEventApi);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CQuickAccess();

	///----------------------------------------------------------------------------------------------------
	/// Render:
	/// 	Renders the Quick Access.
	///----------------------------------------------------------------------------------------------------
	void Render() override;

	///----------------------------------------------------------------------------------------------------
	/// RenderContextMenu:
	/// 	Renders the context menu for a given shortcut.
	///----------------------------------------------------------------------------------------------------
	void RenderContextMenu(const std::string& aIdentifier, const Shortcut_t& aShortcut, bool* aIsActive);

	///----------------------------------------------------------------------------------------------------
	/// AddShortcut:
	/// 	Adds a shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);

	///----------------------------------------------------------------------------------------------------
	/// RemoveShortcut:
	/// 	Removes a shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void RemoveShortcut(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// NotifyShortcut:
	/// 	Adds a notification to a given shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void NotifyShortcut(const char* aIdentifier, const char* aNotificationKey);

	///----------------------------------------------------------------------------------------------------
	/// DenotifyShortcut:
	/// 	Removes a notification to a given shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void DenotifyShortcut(const char* aIdentifier, const char* aNotificationKey);

	///----------------------------------------------------------------------------------------------------
	/// AddContextItem:
	/// 	Adds a context item to the Nexus shortcut.
	///----------------------------------------------------------------------------------------------------
	void AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// AddContextItem:
	/// 	Adds a context item to the given shortcut.
	///----------------------------------------------------------------------------------------------------
	void AddContextItem(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// RemoveContextItem:
	/// 	Removes a context item.
	///----------------------------------------------------------------------------------------------------
	void RemoveContextItem(const char* aIdentifier);
	
	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, Shortcut_t> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// GetOrphanage:
	/// 	Returns a copy of the orphaned callbacks.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, ContextItem_t> GetOrphanage() const;

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all shortcuts and context items matching the address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	///----------------------------------------------------------------------------------------------------
	/// Validate:
	/// 	Validates all shortcuts.
	///----------------------------------------------------------------------------------------------------
	void Validate(bool aLock);

	private:
	NexusLinkData_t*                     NexusLink;
	Mumble::Data*                        MumbleLink;
	CLogApi*                             Logger;
	CInputBindApi*                       InputBindApi;
	CTextureLoader*                      TextureService;
	CLocalization*                       Language;
	CEventApi*                           EventApi;

	mutable std::mutex                   Mutex;
	std::map<std::string, Shortcut_t>    Registry;
	std::map<std::string, ContextItem_t> OrphanedCallbacks;

	float                                Opacity                  = 0.50f;

	Texture_t*                           IconNotification1        = nullptr;
	Texture_t*                           IconNotification2        = nullptr;
	Texture_t*                           IconNotification3        = nullptr;
	Texture_t*                           IconNotification4        = nullptr;
	Texture_t*                           IconNotification5        = nullptr;
	Texture_t*                           IconNotification6        = nullptr;
	Texture_t*                           IconNotification7        = nullptr;
	Texture_t*                           IconNotification8        = nullptr;
	Texture_t*                           IconNotification9        = nullptr;
	Texture_t*                           IconNotificationTooMany  = nullptr;

	///----------------------------------------------------------------------------------------------------
	/// WhereAreMyParents:
	/// 	Returns orphaned context items to their parents.
	///----------------------------------------------------------------------------------------------------
	void WhereAreMyParents();

	///----------------------------------------------------------------------------------------------------
	/// IsValid:
	/// 	Returns true if the passed shortcut is valid. Not threadsafe.
	///----------------------------------------------------------------------------------------------------
	bool IsValid(const Shortcut_t& aShortcut);
};

#endif

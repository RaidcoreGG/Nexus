///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QuickAccess.h
/// Description  :  Contains the logic for the Quick Access HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "Host/GW2/Addons/API/NexusLink.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Core/Logging/LogChannel.h"
#include "Core/Preferences/PrefSetting.h"
#include "Host/GW2/Mumble/MblReader.h"
#include "QaEnum.h"
#include "QaShortcut.h"
#include "UI/Controls/CtlWindow.h"
#include "UI/Services/Localization/LoclApi.h"
#include "UI/Textures/TxLoader.h"
#include "UI/UiRender.h"

constexpr const char* QA_MENU                    = "!Nexus";
constexpr const char* ICON_NEXUS                 = "ICON_NEXUS";
constexpr const char* ICON_NEXUS_HOVER           = "ICON_NEXUS_HOVER";
constexpr const char* ICON_NEXUS_HALLOWEEN       = "ICON_NEXUS_HALLOWEEN";
constexpr const char* ICON_NEXUS_HALLOWEEN_HOVER = "ICON_NEXUS_HALLOWEEN_HOVER";
constexpr const char* ICON_NEXUS_XMAS            = "ICON_NEXUS_XMAS";
constexpr const char* ICON_NEXUS_XMAS_HOVER      = "ICON_NEXUS_XMAS_HOVER";
constexpr const char* QAKEY_GENERIC              = "Generic";

///----------------------------------------------------------------------------------------------------
/// CQuickAccess Class
///----------------------------------------------------------------------------------------------------
class CQuickAccess : public ILogChannel, public virtual IWindow, public virtual IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnAddonStateChanged:
	/// 	Rechecks invalid shortcuts on addon state change.
	///----------------------------------------------------------------------------------------------------
	static void OnAddonStateChanged(void* aEventData);

	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CQuickAccess(CDataLinkApi* aDataLink, CInputBindApi* aInputBindApi, CTextureLoader* aTextureService, CLocalization* aLocalization, CEventApi* aEventApi);

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
	/// PushNotification:
	/// 	Adds a notification to a given shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void PushNotification(const char* aIdentifier, const char* aNotificationKey);

	///----------------------------------------------------------------------------------------------------
	/// PopNotification:
	/// 	Removes a notification to a given shortcut icon.
	///----------------------------------------------------------------------------------------------------
	void PopNotification(const char* aIdentifier, const char* aNotificationKey);

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
	std::map<std::string, CShortcutIcon*> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// GetOrphanage:
	/// 	Returns a copy of the orphaned callbacks.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, ContextItem_t> GetOrphanage() const;

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes all shortcuts and context items matching the address space.
	///----------------------------------------------------------------------------------------------------
	uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	CInputBindApi*                        InputBindApi       {};
	CTextureLoader*                       TextureService     {};
	CLocalization*                        Language           {};
	CEventApi*                            EventApi           {};

	NexusLinkData_t*                      NexusLink          {};
	Mumble::Data*                         MumbleLink         {};

	mutable std::mutex                    Mutex              {};
	std::map<std::string, CShortcutIcon*> Registry           {};
	std::map<std::string, ContextItem_t>  OrphanedCallbacks  {};

	float                                 Opacity            { 0.5f };

	///----------------------------------------------------------------------------------------------------
	/// WhereAreMyParents:
	/// 	Returns orphaned context items to their parents.
	///----------------------------------------------------------------------------------------------------
	void WhereAreMyParents();

	///----------------------------------------------------------------------------------------------------
	/// UpdateNexusLink:
	/// 	Updates the nexus link shortcut icon count.
	///----------------------------------------------------------------------------------------------------
	void UpdateNexusLink();

	///----------------------------------------------------------------------------------------------------
	/// ApplySuppression:
	/// 	Applies the suppressed shortcut list to the registered shortcuts.
	///----------------------------------------------------------------------------------------------------
	void ApplySuppression(const std::vector<std::string>& aSuppressedShortcuts);

	///----------------------------------------------------------------------------------------------------
	/// UpdateMenuIcon:
	/// 	Recreates the menu shortcut with the seasonal or the default icon.
	///----------------------------------------------------------------------------------------------------
	void UpdateMenuIcon(bool aDisableFestiveFlair);

	public:
	/* Declared last: they are destroyed first, before the state their change handlers touch. */
	CSetting<bool>                     VerticalLayout     { "QAVertical", false };
	CSetting<EQaVisibility>            Visibility         { "QAVisibility", EQaVisibility::AlwaysShow };
	CSetting<EQaPosition>              Location           { "QALocation", EQaPosition::Extend };
	CSetting<float>                    OffsetX            { "QAOffsetX", 0.0f };
	CSetting<float>                    OffsetY            { "QAOffsetY", 0.0f };
	CSetting<bool>                     OnlyShowOnHover    { "QAOnlyShowOnHover", false };
	CSetting<std::vector<std::string>> SuppressedShortcuts{ "QASuppressedIcons", {}, [this](const std::vector<std::string>& aSuppressedShortcuts)
	{
		this->ApplySuppression(aSuppressedShortcuts);
	} };
	CSetting<bool>                     NotifyChangelog    { "NotifyChangelog", false };
	CSetting<bool>                     DisableFestiveFlair{ "DisableFestiveFlair", false, [this](const bool& aDisableFestiveFlair)
	{
		this->UpdateMenuIcon(aDisableFestiveFlair);
	} };
};

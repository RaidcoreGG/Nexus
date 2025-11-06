///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaShortcut.h
/// Description  :  Contains the structs holding information about a shortcut.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Core/NexusLink.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "Engine/Loader/Loader.h"
#include "Engine/Textures/TxLoader.h"
#include "Engine/Textures/TxTexture.h"
#include "UI/Services/Localization/LoclApi.h"
#include "UI/UiFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// ContextItem_t Struct
///----------------------------------------------------------------------------------------------------
struct ContextItem_t
{
	std::string ParentID = "";
	GUI_RENDER  Callback = nullptr;
};

///----------------------------------------------------------------------------------------------------
/// CShortcutIcon Class
///----------------------------------------------------------------------------------------------------
class CShortcutIcon : public virtual IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CShortcutIcon(
		std::string aID,
		std::string aIconID,
		std::string aIconHoverID,
		std::string aInputBindID,
		std::string aTooltip
	);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CShortcutIcon();

	///----------------------------------------------------------------------------------------------------
	/// Invalidate:
	/// 	Invalidates the UI component, causing it to refresh.
	///----------------------------------------------------------------------------------------------------
	void Invalidate();

	///----------------------------------------------------------------------------------------------------
	/// Render:
	/// 	Renders the icon.
	/// 	Returns true if the icon is active.
	///----------------------------------------------------------------------------------------------------
	bool Render();

	///----------------------------------------------------------------------------------------------------
	/// AddContextItem:
	/// 	Adds a context menu item.
	///----------------------------------------------------------------------------------------------------
	void AddContextItem(std::string aIdentifier, ContextItem_t aContextItem);

	///----------------------------------------------------------------------------------------------------
	/// RemoveContextItem:
	/// 	Removes a context menu item.
	///----------------------------------------------------------------------------------------------------
	void RemoveContextItem(std::string aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// PushNotifcationSafe:
	/// 	Adds a unique notification key.
	///----------------------------------------------------------------------------------------------------
	void PushNotifcationSafe(std::string aKey);

	///----------------------------------------------------------------------------------------------------
	/// PopNotifcationSafe:
	/// 	Removes a unique notification key.
	///----------------------------------------------------------------------------------------------------
	void PopNotifcationSafe(std::string aKey);

	///----------------------------------------------------------------------------------------------------
	/// IsActive:
	/// 	Returns true if the shortcut is active.
	/// 	Must either have an on-click action or a context menu.
	///----------------------------------------------------------------------------------------------------
	bool IsActive() const;

	///----------------------------------------------------------------------------------------------------
	/// SetSuppression:
	/// 	Sets whether the icon should be suppressed or not.
	///----------------------------------------------------------------------------------------------------
	void SetSuppression(bool aSuppress);

	///----------------------------------------------------------------------------------------------------
	/// GetNotificationKeys:
	/// 	Returns a copy of the notification keys.
	///----------------------------------------------------------------------------------------------------
	std::vector<std::string> GetNotificationKeys() const;

	///----------------------------------------------------------------------------------------------------
	/// GetContextMenuItems:
	/// 	Returns a copy of the context menu items.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, ContextItem_t> GetContextMenuItems() const;

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes all context items matching the address space.
	///----------------------------------------------------------------------------------------------------
	int CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	std::string                          ID             = "";

	CInputBindApi*                       InputBindApi   = nullptr;
	CTextureLoader*                      TextureService = nullptr;
	CLoader*                             Loader         = nullptr;
	CDataLinkApi*                        DataLink       = nullptr;
	CLocalization*                       Language       = nullptr;
	NexusLinkData_t*                     NexusLink      = nullptr;

	bool                                 IsValid        = false;
	bool                                 IsSuppressed   = false;

	std::string                          IconID         = "";
	Texture_t*                           Icon           = nullptr;
	std::string                          IconHoverID    = "";
	Texture_t*                           IconHover      = nullptr;

	std::string                          InputBindID    = "";
	std::string                          IBText         = "";
	std::string                          TooltipText    = "";
	bool                                 IsHovering     = false;

	mutable std::mutex                   Mutex;
	std::vector<std::string>             Notifications;
	std::map<std::string, ContextItem_t> ContextItems;

	enum ETexIdx
	{
		Notify1,
		Notify2,
		Notify3,
		Notify4,
		Notify5,
		Notify6,
		Notify7,
		Notify8,
		Notify9,
		NotifyX,
		HasContextMenu,

		COUNT
	};

	Texture_t* Textures[ETexIdx::COUNT] = {};

	///----------------------------------------------------------------------------------------------------
	/// GetNotificationTexture:
	/// 	Returns the texture for the specified amount of notifications.
	///----------------------------------------------------------------------------------------------------
	Texture_t* GetNotificationTexture(uint32_t aAmount);

	///----------------------------------------------------------------------------------------------------
	/// RenderContextMenu:
	/// 	Renders the context menu associated with the shortcut.
	/// 	Returns true if the context menu is active.
	///----------------------------------------------------------------------------------------------------
	bool RenderContextMenu();

	///----------------------------------------------------------------------------------------------------
	/// PopNotifcation:
	/// 	Removes a unique notification key.
	///----------------------------------------------------------------------------------------------------
	void PopNotifcation(std::string aKey);
};

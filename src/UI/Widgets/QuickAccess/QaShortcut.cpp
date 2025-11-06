///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaShortcut.cpp
/// Description  :  Contains the structs holding information about a shortcut.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "QaShortcut.h"

#include <assert.h>

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "Core/Addons/Addon.h"
#include "Core/Context.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "Resources/ResConst.h"
#include "UI/UiContext.h"

CShortcutIcon::CShortcutIcon(
	std::string aID,
	std::string aIconID,
	std::string aIconHoverID,
	std::string aInputBindID,
	std::string aTooltip
) : IRefCleaner("QuickAccess")
{
	/* FIXME: See Render IsInvalid. */
	CContext* ctx        = CContext::GetContext();
	this->InputBindApi   = ctx->GetInputBindApi();
	this->TextureService = ctx->GetTextureService();
	this->Loader         = ctx->GetLoader();
	this->DataLink       = ctx->GetDataLink();

	this->NexusLink = static_cast<NexusLinkData_t*>(this->DataLink->GetResource(DL_NEXUS_LINK));

	this->ID          = aID;
	this->IconID      = aIconID;
	this->IconHoverID = aIconHoverID;
	this->InputBindID = aInputBindID;
	this->TooltipText = aTooltip;
}

CShortcutIcon::~CShortcutIcon()
{
}

void CShortcutIcon::Invalidate()
{
	this->IsValid = false;
}

bool CShortcutIcon::Render()
{
	if (!this->IsActive())  { return false; }
	if (this->IsSuppressed) { return false; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (!this->IsValid)
	{
		/* FIXME: Since QuickAccess is created within UiContext ctor, the first shorcut is also created within that ctor.
		 * Thus deadlocking. Hence we move the UIctx init into here. */
		CContext*   ctx   = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();
		this->Language    = uictx->GetLocalization();

		if (!this->InputBindID.empty())
		{
			const InputBind_t* ib = this->InputBindApi->Get(this->InputBindID);

			if (ib != nullptr && ib->Device != EInputDevice::None)
			{
				this->IBText = IBToString(*ib, true);
			}
		}

		this->IsValid = true;
	}

	if (!(this->Icon && this->IconHover))
	{
		/* Get requested textures. */
		this->Icon      = this->TextureService->Get(this->IconID.c_str());
		this->IconHover = this->TextureService->Get(this->IconHoverID.c_str());

		return false;
	}

	ImGui::BeginGroup();

	/* Push icon styles. */
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.f, 1.f) * this->NexusLink->Scaling);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));

	ImVec2 baseSizeUnscaled   = ImVec2(32.f, 32.f); // Shortcut icon
	ImVec2 notifySizeUnscaled = ImVec2(16.f, 16.f); // Notification icon
	ImVec2 ctxSizeUnscaled    = ImVec2(12.f, 12.f); // Context menu available icon

	ImVec2 iconPos   = ImGui::GetCursorPos();
	ImVec2 notifyPos = iconPos + ((baseSizeUnscaled - notifySizeUnscaled) * this->NexusLink->Scaling);
	ImVec2 ctxPos    = ImVec2(
		iconPos.x + ((baseSizeUnscaled.x - ctxSizeUnscaled.x) / 2.f * this->NexusLink->Scaling),
		iconPos.y + ((baseSizeUnscaled.y - (ctxSizeUnscaled.y / 2.f)) * this->NexusLink->Scaling)
	);

	bool iconActive = false;
	Texture_t* icon = !this->IsHovering ? this->Icon : this->IconHover;
	if (ImGui::ImageButton(icon->Resource, baseSizeUnscaled * this->NexusLink->Scaling))
	{
		this->PopNotifcation(QAKEY_GENERIC);
		this->InputBindApi->Invoke(this->InputBindID);
	}
	iconActive = ImGui::IsItemHovered() || ImGui::IsItemClicked();

	Texture_t* notificationIcon = this->GetNotificationTexture(static_cast<uint32_t>(this->Notifications.size()));
	if (notificationIcon)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::SetCursorPos(notifyPos);
		ImGui::Image(notificationIcon->Resource, notifySizeUnscaled * this->NexusLink->Scaling);
		ImGui::PopItemFlag();
	}

	if (this->ContextItems.size() > 0)
	{
		/* ensure all textures */
		if (!this->Textures[ETexIdx::HasContextMenu])
		{
			CContext* ctx = CContext::GetContext();
			this->Textures[ETexIdx::HasContextMenu] = this->TextureService->GetOrCreate("ICON_CONTEXTMENU", RES_ICON_CONTEXTMENU_AVAILABLE, ctx->GetModule());
		}
		else
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::SetCursorPos(ctxPos);
			ImGui::Image(this->Textures[ETexIdx::HasContextMenu]->Resource, ctxSizeUnscaled * this->NexusLink->Scaling);
			ImGui::PopItemFlag();
		}
	}

	/* Pop icon styles. */
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::EndGroup();

	this->IsHovering = iconActive;
	if (ImGui::IsItemHovered() && !this->TooltipText.empty())
	{
		ImGui::BeginTooltip();
		if (this->IBText.empty())
		{
			ImGui::Text(this->Language->Translate(this->TooltipText.c_str()));
		}
		else
		{
			ImGui::Text("%s [%s]", this->Language->Translate(this->TooltipText.c_str()), this->IBText.c_str());
		}
		if (this->ContextItems.size() > 0)
		{
			ImGui::TextDisabled(this->Language->Translate("((000102))"));
		}
		ImGui::EndTooltip();
	}

	bool ctxMenuActive = this->RenderContextMenu();

	return iconActive || ctxMenuActive;
}

void CShortcutIcon::AddContextItem(std::string aIdentifier, ContextItem_t aContextItem)
{
	assert(aContextItem.Callback);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->ContextItems.find(aIdentifier);

	if (it == this->ContextItems.end())
	{
		this->ContextItems.emplace(aIdentifier, aContextItem);
	}
}

void CShortcutIcon::RemoveContextItem(std::string aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->ContextItems.erase(aIdentifier);
}

void CShortcutIcon::PushNotifcationSafe(std::string aKey)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto notifIt = std::find(this->Notifications.begin(), this->Notifications.end(), aKey);

	if (notifIt == this->Notifications.end())
	{
		this->Notifications.push_back(aKey);
	}
}

void CShortcutIcon::PopNotifcationSafe(std::string aKey)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->PopNotifcation(aKey);
}

bool CShortcutIcon::IsActive() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	bool hasContextMenu = this->ContextItems.size() > 0;
	bool hasClickHandler = this->InputBindApi->HasHandler(this->InputBindID);

	/* Needs to have either a context menu or click handler. */
	return hasContextMenu || hasClickHandler;
}

void CShortcutIcon::SetSuppression(bool aSuppress)
{
	this->IsSuppressed = aSuppress;
}

std::vector<std::string> CShortcutIcon::GetNotificationKeys() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	return this->Notifications;
}

std::map<std::string, ContextItem_t> CShortcutIcon::GetContextMenuItems() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	return this->ContextItems;
}

int CShortcutIcon::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto ctxItem = this->ContextItems.begin(); ctxItem != this->ContextItems.end();)
	{
		GUI_RENDER callback = ctxItem->second.Callback;
		if (callback >= aStartAddress && callback <= aEndAddress)
		{
			refCounter++;
			ctxItem = this->ContextItems.erase(ctxItem);
		}
		else
		{
			ctxItem++;
		}
	}

	return refCounter;
}

Texture_t* CShortcutIcon::GetNotificationTexture(uint32_t aAmount)
{
	if (aAmount == 0) { return nullptr; }

	Texture_t* icon = nullptr;

	switch (aAmount)
	{
		case 1:  icon = this->Textures[ETexIdx::Notify1]; break;
		case 2:  icon = this->Textures[ETexIdx::Notify2]; break;
		case 3:  icon = this->Textures[ETexIdx::Notify3]; break;
		case 4:  icon = this->Textures[ETexIdx::Notify4]; break;
		case 5:  icon = this->Textures[ETexIdx::Notify5]; break;
		case 6:  icon = this->Textures[ETexIdx::Notify6]; break;
		case 7:  icon = this->Textures[ETexIdx::Notify7]; break;
		case 8:  icon = this->Textures[ETexIdx::Notify8]; break;
		case 9:  icon = this->Textures[ETexIdx::Notify9]; break;
		default: icon = this->Textures[ETexIdx::NotifyX]; break;
	}

	if (!icon)
	{
		CContext* ctx = CContext::GetContext();

		switch (aAmount)
		{
			case 1:  icon = this->Textures[ETexIdx::Notify1] = this->TextureService->GetOrCreate("QA_NOTIFY1", RES_ICON_NOTIFICATION1, ctx->GetModule()); break;
			case 2:  icon = this->Textures[ETexIdx::Notify2] = this->TextureService->GetOrCreate("QA_NOTIFY2", RES_ICON_NOTIFICATION2, ctx->GetModule()); break;
			case 3:  icon = this->Textures[ETexIdx::Notify3] = this->TextureService->GetOrCreate("QA_NOTIFY3", RES_ICON_NOTIFICATION3, ctx->GetModule()); break;
			case 4:  icon = this->Textures[ETexIdx::Notify4] = this->TextureService->GetOrCreate("QA_NOTIFY4", RES_ICON_NOTIFICATION4, ctx->GetModule()); break;
			case 5:  icon = this->Textures[ETexIdx::Notify5] = this->TextureService->GetOrCreate("QA_NOTIFY5", RES_ICON_NOTIFICATION5, ctx->GetModule()); break;
			case 6:  icon = this->Textures[ETexIdx::Notify6] = this->TextureService->GetOrCreate("QA_NOTIFY6", RES_ICON_NOTIFICATION6, ctx->GetModule()); break;
			case 7:  icon = this->Textures[ETexIdx::Notify7] = this->TextureService->GetOrCreate("QA_NOTIFY7", RES_ICON_NOTIFICATION7, ctx->GetModule()); break;
			case 8:  icon = this->Textures[ETexIdx::Notify8] = this->TextureService->GetOrCreate("QA_NOTIFY8", RES_ICON_NOTIFICATION8, ctx->GetModule()); break;
			case 9:  icon = this->Textures[ETexIdx::Notify9] = this->TextureService->GetOrCreate("QA_NOTIFY9", RES_ICON_NOTIFICATION9, ctx->GetModule()); break;
			default: icon = this->Textures[ETexIdx::NotifyX] = this->TextureService->GetOrCreate("QA_NOTIFYX", RES_ICON_NOTIFICATIONX, ctx->GetModule()); break;
		}
	}

	return icon;
}

bool CShortcutIcon::RenderContextMenu()
{
	if (this->ContextItems.size() == 0)
	{
		return false;
	}

	static const char* ID_CONTEXTMENU = "QuickAccess::ContextMenu::";
	std::string ctxMenuID = ID_CONTEXTMENU + this->ID;

	bool drawingCtxMenu = false;

	if (ImGui::BeginPopupContextItem(ctxMenuID.c_str()))
	{
		drawingCtxMenu = true;

		/* Helpers to draw separators between callbacks. */
		size_t amtItems = this->ContextItems.size();
		size_t idx = 0;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f)); /* Small checkboxes. */
		for (auto& [identifier, contextitem] : this->ContextItems)
		{
			idx++;

			if (contextitem.Callback)
			{
				IAddon* iaddon = this->Loader->GetOwner(contextitem.Callback);
				CAddon* addon = dynamic_cast<CAddon*>(iaddon);

				assert(addon);

				/* Addon name header. */
				ImGui::TextDisabled(addon->GetName().c_str());

				/* Addon content callback. */
				contextitem.Callback();

				/* Separator between addons. */
				if (idx != amtItems)
				{
					ImGui::Separator();
				}
			}
		}
		ImGui::PopStyleVar(1);

		ImGui::EndPopup();
	}
	ImGui::OpenPopupOnItemClick(ctxMenuID.c_str(), 1);

	return drawingCtxMenu;
}

void CShortcutIcon::PopNotifcation(std::string aKey)
{
	auto notifIt = std::find(this->Notifications.begin(), this->Notifications.end(), aKey);

	if (notifIt != this->Notifications.end())
	{
		this->Notifications.erase(notifIt);
	}
}

///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QuickAccess.cpp
/// Description  :  Contains the logic for the Quick Access HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "QuickAccess.h"

#include <vector>

#include "ImAnimate/ImAnimate.h"
#include "imgui/imgui_extensions.h"
#include "imgui/imgui_internal.h"

#include "Core/Addons/AddConst.h"
#include "Core/Addons/Addon.h"
#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"
#include "Core/Preferences/PrefContext.h"
#include "Engine/Cleanup/RefCleanerContext.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "Resources/ResConst.h"
#include "UI/UIContext.h"

#define GW2_QUICKACCESS_ITEMS 10;

float size = 32.0f;

void CQuickAccess::OnAddonStateChanged(void* aEventData)
{
	CContext*     ctx   = CContext::GetContext();
	CUiContext*   uictx = ctx->GetUIContext();
	CQuickAccess* qactx = uictx->GetQuickAccess();

	qactx->ValidateSafe();
}

CQuickAccess::CQuickAccess(CDataLinkApi* aDataLink, CLogApi* aLogger, CInputBindApi* aInputBindApi, CTextureLoader* aTextureService, CLocalization* aLocalization, CEventApi* aEventApi)
{
	this->NexusLink      = (NexusLinkData_t*)aDataLink->GetResource(DL_NEXUS_LINK);
	this->MumbleLink     = (Mumble::Data*)aDataLink->GetResource(DL_MUMBLE_LINK);
	this->Logger         = aLogger;
	this->InputBindApi   = aInputBindApi;
	this->TextureService = aTextureService;
	this->Language       = aLocalization;
	this->EventApi       = aEventApi;

	CContext*  ctx         = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	this->VerticalLayout     = settingsctx->Get<bool>(OPT_QAVERTICAL, false);
	this->Location           = settingsctx->Get<EQaPosition>(OPT_QALOCATION, EQaPosition::Extend);
	this->Offset.x           = settingsctx->Get<float>(OPT_QAOFFSETX, 0.0f);
	this->Offset.y           = settingsctx->Get<float>(OPT_QAOFFSETY, 0.0f);
	this->Visibility         = settingsctx->Get<EQaVisibility>(OPT_QAVISIBILITY, EQaVisibility::AlwaysShow);

	this->EventApi->Subscribe(EV_ADDON_LOADED,   CQuickAccess::OnAddonStateChanged);
	this->EventApi->Subscribe(EV_ADDON_UNLOADED, CQuickAccess::OnAddonStateChanged);

	CRefCleanerContext::Get()->Register("CQuickAccess", this);
}

CQuickAccess::~CQuickAccess()
{
	this->EventApi->Unsubscribe(EV_ADDON_LOADED,   CQuickAccess::OnAddonStateChanged);
	this->EventApi->Unsubscribe(EV_ADDON_UNLOADED, CQuickAccess::OnAddonStateChanged);
}

void CQuickAccess::Render()
{
	if (this->IsInvalid)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		for (auto& [identifier, shortcut] : this->Registry)
		{
			const InputBind_t& ib = *this->InputBindApi->Get(shortcut.IBIdentifier);
			if (ib.Device != EInputDevice::None)
			{
				shortcut.IBText = IBToString(ib, true);
			}
			else
			{
				shortcut.IBText.clear();
			}
		}

		this->IsInvalid = false;
	}

	this->NexusLink->QuickAccessMode = (int)this->Location;
	this->NexusLink->QuickAccessIsVertical = this->VerticalLayout;

	switch (this->Visibility)
	{
		/* continue rendering */
		default:
		case EQaVisibility::AlwaysShow: { break; }

		case EQaVisibility::Gameplay:
		{
			/* don't render if not gameplay */
			if (!this->NexusLink->IsGameplay)
			{
				return;
			}

			break;
		}
		case EQaVisibility::OutOfCombat:
		{
			/* don't render if not gameplay */
			if (!this->NexusLink->IsGameplay)
			{
				return;
			}

			/* don't render if in combat */
			if (this->MumbleLink->Context.IsInCombat)
			{
				return;
			}

			break;
		}
		case EQaVisibility::InCombat:
		{
			/* don't render if not gameplay */
			if (!this->NexusLink->IsGameplay)
			{
				return;
			}

			/* don't render if out of combat */
			if (!this->MumbleLink->Context.IsInCombat)
			{
				return;
			}

			break;
		}

		/* don't render*/
		case EQaVisibility::Hide: { return; }
	}

	bool isActive = false;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, this->Opacity);

	ImVec2 pos = ImVec2(0.0f, 0.0f);

	CContext*        ctx      = CContext::GetContext();
	RenderContext_t* renderer = ctx->GetRendererCtx();

	switch (this->Location)
	{
		case EQaPosition::Extend:
			pos.x += (size * UIRoot::ScalingFactor) * GW2_QUICKACCESS_ITEMS;
			break;
		case EQaPosition::Under:
			pos.y += size * UIRoot::ScalingFactor;
			break;
		case EQaPosition::Bottom:
			pos.y += renderer->Window.Height - (size * 2 * UIRoot::ScalingFactor);
			break;
	}

	pos.x += this->Offset.x;
	pos.y += this->Offset.y;

	ImGui::SetNextWindowPos(pos);
	if (ImGui::Begin("QuickAccess", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar))
	{
		bool menuFound = false;

		unsigned c = 0;
		const std::lock_guard<std::mutex> lock(this->Mutex);
		for (auto& [identifier, shortcut] : this->Registry)
		{
			if (!shortcut.IsValid) { continue; }

			//Logger->Debug(CH_QUICKACCESS, "size: %f | c: %d | scale: %f", size, c, Renderer::Scaling);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
			if (this->VerticalLayout)
			{
				ImGui::SetCursorPos(ImVec2(0, ((size * c) + (c ? 1 : 0)) * UIRoot::ScalingFactor));
			}
			else
			{
				ImGui::SetCursorPos(ImVec2(((size * c) + (c ? 1 : 0)) * UIRoot::ScalingFactor, 0));
			}

			ImVec2 pos = ImGui::GetCursorPos();

			bool iconHovered = false;

			if (shortcut.TextureNormal && shortcut.TextureNormal->Resource &&
				shortcut.TextureHover && shortcut.TextureHover->Resource)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
				if (ImGui::IconButton(!shortcut.IsHovering ? shortcut.TextureNormal->Resource : shortcut.TextureHover->Resource, ImVec2(size * UIRoot::ScalingFactor, size * UIRoot::ScalingFactor)))
				{
					isActive = true;
					if (shortcut.IBIdentifier.length() > 0)
					{
						// call this async because we're currently iterating the list
						std::thread([this, identifier]()
						{
							this->PopNotification(identifier.c_str(), "Generic");
						}).detach();
						this->InputBindApi->Invoke(shortcut.IBIdentifier);
					}
				}
				ImGui::PopStyleVar();
				iconHovered = ImGui::IsItemHovered() || ImGui::IsItemClicked();
			}
			else if (shortcut.TextureGetAttempts < 10)
			{
				shortcut.TextureNormal = this->TextureService->Get(shortcut.TextureNormalIdentifier.c_str());
				shortcut.TextureHover = this->TextureService->Get(shortcut.TextureHoverIdentifier.c_str());
				shortcut.TextureGetAttempts++;
			}
			else
			{
				this->Logger->Warning(CH_QUICKACCESS, "Cancelled getting textures for shortcut \"%s\" after 10 failed attempts.", identifier.c_str());

				/* fallback icons */
				shortcut.TextureNormal = this->TextureService->Get(ICON_GENERIC);
				shortcut.TextureHover = this->TextureService->Get(ICON_GENERIC_HOVER);

				/* absolute sanity check */
				if (shortcut.TextureNormal == nullptr || shortcut.TextureHover == nullptr)
				{
					this->Logger->Warning(CH_QUICKACCESS, "Neither promised textures nor fallback textures could be loaded, removing shortcut \"%s\".", identifier.c_str());

					// call this async because we're currently iterating the list
					std::thread([this, identifier]()
					{
						this->RemoveShortcut(identifier.c_str());
					}).detach();
				}
			}

			bool notifHovered = false;

			if (shortcut.Notifications.size() > 0)
			{
				Texture_t* icon = nullptr;
				switch (shortcut.Notifications.size())
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
					switch (shortcut.Notifications.size())
					{
						case 1:  this->Textures[ETexIdx::Notify1] = this->TextureService->GetOrCreate("QA_NOTIFY1", RES_ICON_NOTIFICATION1, ctx->GetModule()); break;
						case 2:  this->Textures[ETexIdx::Notify2] = this->TextureService->GetOrCreate("QA_NOTIFY2", RES_ICON_NOTIFICATION2, ctx->GetModule()); break;
						case 3:  this->Textures[ETexIdx::Notify3] = this->TextureService->GetOrCreate("QA_NOTIFY3", RES_ICON_NOTIFICATION3, ctx->GetModule()); break;
						case 4:  this->Textures[ETexIdx::Notify4] = this->TextureService->GetOrCreate("QA_NOTIFY4", RES_ICON_NOTIFICATION4, ctx->GetModule()); break;
						case 5:  this->Textures[ETexIdx::Notify5] = this->TextureService->GetOrCreate("QA_NOTIFY5", RES_ICON_NOTIFICATION5, ctx->GetModule()); break;
						case 6:  this->Textures[ETexIdx::Notify6] = this->TextureService->GetOrCreate("QA_NOTIFY6", RES_ICON_NOTIFICATION6, ctx->GetModule()); break;
						case 7:  this->Textures[ETexIdx::Notify7] = this->TextureService->GetOrCreate("QA_NOTIFY7", RES_ICON_NOTIFICATION7, ctx->GetModule()); break;
						case 8:  this->Textures[ETexIdx::Notify8] = this->TextureService->GetOrCreate("QA_NOTIFY8", RES_ICON_NOTIFICATION8, ctx->GetModule()); break;
						case 9:  this->Textures[ETexIdx::Notify9] = this->TextureService->GetOrCreate("QA_NOTIFY9", RES_ICON_NOTIFICATION9, ctx->GetModule()); break;
						default: this->Textures[ETexIdx::NotifyX] = this->TextureService->GetOrCreate("QA_NOTIFYX", RES_ICON_NOTIFICATIONTOOMANY, ctx->GetModule()); break;
					}
				}
				else
				{
					float offIcon = (size * UIRoot::ScalingFactor) / 2.0f;

					ImVec2 notificationPos = pos;

					notificationPos.x += offIcon;
					notificationPos.y += offIcon;

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::SetCursorPos(notificationPos);
					ImGui::Image(icon->Resource, ImVec2(offIcon, offIcon));
					ImGui::PopItemFlag();
					notifHovered = ImGui::IsItemHovered();
				}
			}

			if (shortcut.ContextItems.size() > 0)
			{
				/* ensure all textures */
				if (!this->Textures[ETexIdx::HasContextMenu])
				{
					CContext* ctx = CContext::GetContext();
					this->Textures[ETexIdx::HasContextMenu] = this->TextureService->GetOrCreate("ICON_CONTEXTMENU", RES_ICON_CONTEXTMENU_AVAILABLE, ctx->GetModule());
				}
				else
				{
					/* 24.f is the size of the context menu arrow. */
					float ctxIconSize = 24.f * UIRoot::ScalingFactor;

					ImVec2 contextArrowPos = pos;

					contextArrowPos.x += ((size - 24.f) * UIRoot::ScalingFactor) / 2.0f; // center horizontally
					contextArrowPos.y += (size * UIRoot::ScalingFactor) - (24.f / 2.0f);

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::SetCursorPos(contextArrowPos);
					ImGui::Image(this->Textures[ETexIdx::HasContextMenu]->Resource, ImVec2(ctxIconSize, ctxIconSize));
					ImGui::PopItemFlag();
				}
			}

			ImGui::PopStyleVar();
			shortcut.IsHovering = iconHovered || notifHovered;
			if (shortcut.TooltipText.length() > 0)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					if (shortcut.IBText.empty())
					{
						ImGui::Text(this->Language->Translate(shortcut.TooltipText.c_str()));
					}
					else
					{
						ImGui::Text("%s [%s]", this->Language->Translate(shortcut.TooltipText.c_str()), shortcut.IBText.c_str());
					}
					if (shortcut.ContextItems.size() > 0)
					{
						ImGui::TextDisabled(this->Language->Translate("((000102))"));
					}
					ImGui::EndTooltip();
				}
			}

			c++;

			this->RenderContextMenu(identifier, shortcut, &isActive);
		}

		bool isHoveringNative = false;
		if (this->Location == EQaPosition::Extend)
		{
			ImVec2 mPos = ImGui::GetMousePos();
			if (mPos.x != -FLT_MAX && mPos.y != -FLT_MAX && mPos.x < pos.x - this->Offset.x && mPos.y < UIRoot::ScalingFactor * size)
			{
				isHoveringNative = true;
			}
		}

		bool isHovering = ImGui::IsWindowHovered() || isActive || isHoveringNative;

		if (isHovering)
		{
			ImGui::Animate(0.5f, 1, 350, &this->Opacity, ImAnimate::ECurve::Linear);
		}
		else
		{
			ImGui::Animate(1, 0.5f, 350, &this->Opacity, ImAnimate::ECurve::Linear);
		}
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void CQuickAccess::RenderContextMenu(const std::string& aIdentifier, const Shortcut_t& aShortcut, bool* aIsActive)
{
	static CContext* ctx = CContext::GetContext();
	static CLoader* loader = ctx->GetLoader();

	if (aShortcut.ContextItems.size() > 0)
	{
		std::string ctxId = "ShortcutsCtxMenu##" + aIdentifier;
		if (ImGui::BeginPopupContextItem(ctxId.c_str()))
		{
			*aIsActive = true;

			size_t amtItems = aShortcut.ContextItems.size();
			size_t idx = 0;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
			for (auto& [cbidentifier, cbshortcut] : aShortcut.ContextItems)
			{
				idx++;

				if (cbshortcut.Callback)
				{
					IAddon* iaddon = loader->GetOwner(cbshortcut.Callback);
					CAddon* addon = dynamic_cast<CAddon*>(iaddon);
					ImGui::TextDisabled(addon->GetName().c_str());
					cbshortcut.Callback();

					if (idx != amtItems)
					{
						ImGui::Separator();
					}
				}
			}
			ImGui::PopStyleVar();

			ImGui::EndPopup();
		}
		ImGui::OpenPopupOnItemClick(ctxId.c_str(), 1);
	}
}

void CQuickAccess::AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText)
{
	if (!aIdentifier)             { return; }
	if (!aTextureIdentifier)      { return; }
	if (!aTextureHoverIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (this->Registry.find(aIdentifier) == this->Registry.end())
	{
		Texture_t* normal = this->TextureService->Get(aTextureIdentifier);
		Texture_t* hover = this->TextureService->Get(aTextureHoverIdentifier);
		Shortcut_t sh{};
		sh.TextureNormalIdentifier = aTextureIdentifier;
		sh.TextureHoverIdentifier = aTextureHoverIdentifier;
		sh.TextureNormal = normal;
		sh.TextureHover = hover;
		sh.IBIdentifier = aInputBindIdentifier;

		const InputBind_t* ib = this->InputBindApi->Get(aInputBindIdentifier);

		if (ib != nullptr && ib->Device != EInputDevice::None)
		{
			sh.IBText = IBToString(*ib, true);
		}

		sh.TooltipText = aTooltipText;
		sh.TextureGetAttempts = 0;
		this->Registry[aIdentifier] = sh;

		int amt = 0;
		if (sh.TextureNormal != nullptr) { amt++; }
		if (sh.TextureHover != nullptr) { amt++; }
	}

	/* Check if this shortcut can maybe adopt some orphans. */
	this->WhereAreMyParents();
	this->Validate();
}

void CQuickAccess::RemoveShortcut(const char* aIdentifier)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		for (auto& orphan : it->second.ContextItems)
		{
			this->OrphanedCallbacks[orphan.first] = orphan.second;
		}
	}

	this->Registry.erase(aIdentifier);

	this->Validate();
}

void CQuickAccess::PushNotification(const char* aIdentifier, const char* aNotificationKey)
{
	if (!aIdentifier)      { return; }
	if (!aNotificationKey) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		auto notifIt = std::find(it->second.Notifications.begin(), it->second.Notifications.end(), aNotificationKey);

		if (notifIt != it->second.Notifications.end())
		{
			it->second.Notifications.push_back(aNotificationKey);
		}
	}
}

void CQuickAccess::PopNotification(const char* aIdentifier, const char* aNotificationKey)
{
	if (!aIdentifier)      { return; }
	if (!aNotificationKey) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		auto notifIt = std::find(it->second.Notifications.begin(), it->second.Notifications.end(), aNotificationKey);

		if (notifIt != it->second.Notifications.end())
		{
			it->second.Notifications.erase(notifIt);
		}
	}
}

void CQuickAccess::AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback)
{
	if (!aIdentifier)             { return; }
	if (!aShortcutRenderCallback) { return; }

	this->AddContextItem(aIdentifier, QA_MENU, aShortcutRenderCallback);
}

void CQuickAccess::AddContextItem(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback)
{
	if (!aIdentifier)               { return; }
	if (!aTargetShortcutIdentifier) { return; }
	if (!aShortcutRenderCallback)   { return; }

	ContextItem_t contextItem{
		aTargetShortcutIdentifier,
		aShortcutRenderCallback
	};

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto parentIt = this->Registry.find(aTargetShortcutIdentifier);

	if (parentIt != this->Registry.end())
	{
		auto ctxItemIt = parentIt->second.ContextItems.find(aIdentifier);

		if (ctxItemIt == parentIt->second.ContextItems.end())
		{
			parentIt->second.ContextItems.emplace(aIdentifier, contextItem);
		}
		else
		{
			this->Logger->Warning(
				CH_QUICKACCESS,
				"Context menu item already registered: %s (Parent: \"%s\")",
				aIdentifier,
				aTargetShortcutIdentifier
			);
		}
	}
	else
	{
		auto orphanIt = this->OrphanedCallbacks.find(aIdentifier);

		if (orphanIt == this->OrphanedCallbacks.end())
		{
			this->OrphanedCallbacks.emplace(aIdentifier, contextItem);
		}
		else
		{
			this->Logger->Warning(
				CH_QUICKACCESS,
				"Context menu item already registered: %s (Parent: \"%s\")",
				aIdentifier,
				aTargetShortcutIdentifier
			);
		}
	}

	this->Validate();
}

void CQuickAccess::RemoveContextItem(const char* aIdentifier)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, shortcut] : this->Registry)
	{
		shortcut.ContextItems.erase(aIdentifier);
	}

	this->OrphanedCallbacks.erase(aIdentifier);

	this->Validate();
}

std::map<std::string, Shortcut_t> CQuickAccess::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	return this->Registry;
}

std::map<std::string, ContextItem_t> CQuickAccess::GetOrphanage() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	return this->OrphanedCallbacks;
}

int CQuickAccess::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* Remove children from parents. */
	for (auto& [identifier, shortcut] : this->Registry)
	{
		for (auto ctxItem = shortcut.ContextItems.begin(); ctxItem != shortcut.ContextItems.end();)
		{
			GUI_RENDER callback = ctxItem->second.Callback;
			if (callback >= aStartAddress && callback <= aEndAddress)
			{
				refCounter++;
				ctxItem = shortcut.ContextItems.erase(ctxItem);
			}
			else
			{
				ctxItem++;
			}
		}
	}

	/* Remove bastard children. */
	for (auto orphanIt = this->OrphanedCallbacks.begin(); orphanIt != this->OrphanedCallbacks.end();)
	{
		GUI_RENDER callback = orphanIt->second.Callback;
		if (callback >= aStartAddress && callback <= aEndAddress)
		{
			refCounter++;
			orphanIt = this->OrphanedCallbacks.erase(orphanIt);
		}
		else
		{
			orphanIt++;
		}
	}

	this->Validate();

	return refCounter;
}

void CQuickAccess::ValidateSafe()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Validate();
}

void CQuickAccess::WhereAreMyParents()
{
	for (auto orphanIt = this->OrphanedCallbacks.begin(); orphanIt != this->OrphanedCallbacks.end();)
	{
		/* Check if a shortcut exists, that matches the orphan's target. */
		auto parentIt = this->Registry.find(orphanIt->second.ParentID);

		/* If we do have a parent. */
		if (parentIt != this->Registry.end())
		{
			/* Sanity check that we're not a duplicate child. */
			if (parentIt->second.ContextItems.find(orphanIt->first) == parentIt->second.ContextItems.end())
			{
				parentIt->second.ContextItems.insert({ orphanIt->first, orphanIt->second });
			}

			/* Annihilate the orphan. */
			orphanIt = this->OrphanedCallbacks.erase(orphanIt);
		}
		else
		{
			/* Still an orphan :( */
			orphanIt++;
		}
	}
}

bool CQuickAccess::IsValid(const Shortcut_t& aShortcut)
{
	/* Has context menu. */
	if (aShortcut.ContextItems.size() > 0)
	{
		return true;
	}

	/* Has click action. */
	if (this->InputBindApi->HasHandler(aShortcut.IBIdentifier))
	{
		return true;
	}

	/* Neither context menu, nor click action. It's just an icon -> Invalid. */
	return false;
}

void CQuickAccess::Validate()
{
	int amtValid = 0;

	for (auto& [identifier, shortcut] : this->Registry)
	{
		shortcut.IsValid = this->IsValid(shortcut);

		if (shortcut.IsValid)
		{
			amtValid++;
		}
	}

	this->NexusLink->QuickAccessIconsCount = amtValid;
}

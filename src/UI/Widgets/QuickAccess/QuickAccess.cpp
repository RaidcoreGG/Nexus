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

#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"
#include "Engine/Inputs/InputBinds/IbConst.h"
#include "Engine/Loader/LdrConst.h"
#include "Core/Preferences/PrefContext.h"
#include "Resources/ResConst.h"
#include "UI/UIContext.h"

#define GW2_QUICKACCESS_ITEMS 10;

float size = 32.0f;

void CQuickAccess::OnAddonLoaded(void* aEventData)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();
	CQuickAccess* qactx = uictx->GetQuickAccess();

	qactx->Validate(true);
}

CQuickAccess::CQuickAccess(CDataLinkApi* aDataLink, CLogApi* aLogger, CInputBindApi* aInputBindApi, CTextureLoader* aTextureService, CLocalization* aLocalization, CEventApi* aEventApi)
{
	this->NexusLink = (NexusLinkData_t*)aDataLink->GetResource(DL_NEXUS_LINK);
	this->MumbleLink = (Mumble::Data*)aDataLink->GetResource(DL_MUMBLE_LINK);
	this->Logger = aLogger;
	this->InputBindApi = aInputBindApi;
	this->TextureService = aTextureService;
	this->Language = aLocalization;
	this->EventApi = aEventApi;

	CContext* ctx = CContext::GetContext();
	CSettings* settingsCtx = ctx->GetSettingsCtx();

	this->VerticalLayout = settingsCtx->Get<bool>(OPT_QAVERTICAL, false);
	this->ShowArcDPSShortcut = settingsCtx->Get<bool>(OPT_QASHOWARCDPS, true);
	this->Location = settingsCtx->Get<EQaPosition>(OPT_QALOCATION, EQaPosition::Extend);
	this->Offset.x = settingsCtx->Get<float>(OPT_QAOFFSETX, 0.0f);
	this->Offset.y = settingsCtx->Get<float>(OPT_QAOFFSETY, 0.0f);
	this->Visibility = settingsCtx->Get<EQaVisibility>(OPT_QAVISIBILITY, EQaVisibility::AlwaysShow);

	this->EventApi->Subscribe(EV_ADDON_LOADED, CQuickAccess::OnAddonLoaded);
}

CQuickAccess::~CQuickAccess()
{
	this->EventApi->Unsubscribe(EV_ADDON_LOADED, CQuickAccess::OnAddonLoaded);

	this->NexusLink = nullptr;
	this->MumbleLink = nullptr;
	this->Logger = nullptr;
	this->InputBindApi = nullptr;
	this->TextureService = nullptr;
	this->Language = nullptr;
	this->EventApi = nullptr;
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
							this->DenotifyShortcut(identifier.c_str(), "Generic");
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
				/* ensure all textures */
				if (!(this->IconNotification1 &&
					this->IconNotification2 &&
					this->IconNotification3 &&
					this->IconNotification4 &&
					this->IconNotification5 &&
					this->IconNotification6 &&
					this->IconNotification7 &&
					this->IconNotification8 &&
					this->IconNotification9 &&
					this->IconNotificationTooMany))
				{
					CContext* ctx = CContext::GetContext();
					this->IconNotification1 = this->TextureService->GetOrCreate("ICON_NOTIFICATION1", RES_ICON_NOTIFICATION1, ctx->GetModule());
					this->IconNotification2 = this->TextureService->GetOrCreate("ICON_NOTIFICATION2", RES_ICON_NOTIFICATION2, ctx->GetModule());
					this->IconNotification3 = this->TextureService->GetOrCreate("ICON_NOTIFICATION3", RES_ICON_NOTIFICATION3, ctx->GetModule());
					this->IconNotification4 = this->TextureService->GetOrCreate("ICON_NOTIFICATION4", RES_ICON_NOTIFICATION4, ctx->GetModule());
					this->IconNotification5 = this->TextureService->GetOrCreate("ICON_NOTIFICATION5", RES_ICON_NOTIFICATION5, ctx->GetModule());
					this->IconNotification6 = this->TextureService->GetOrCreate("ICON_NOTIFICATION6", RES_ICON_NOTIFICATION6, ctx->GetModule());
					this->IconNotification7 = this->TextureService->GetOrCreate("ICON_NOTIFICATION7", RES_ICON_NOTIFICATION7, ctx->GetModule());
					this->IconNotification8 = this->TextureService->GetOrCreate("ICON_NOTIFICATION8", RES_ICON_NOTIFICATION8, ctx->GetModule());
					this->IconNotification9 = this->TextureService->GetOrCreate("ICON_NOTIFICATION9", RES_ICON_NOTIFICATION9, ctx->GetModule());
					this->IconNotificationTooMany = this->TextureService->GetOrCreate("ICON_NOTIFICATIONTOOMANY", RES_ICON_NOTIFICATIONTOOMANY, ctx->GetModule());
				}
				else
				{
					Texture_t* icon = nullptr;
					switch (shortcut.Notifications.size())
					{
						case 1: icon = this->IconNotification1; break;
						case 2: icon = this->IconNotification2; break;
						case 3: icon = this->IconNotification3; break;
						case 4: icon = this->IconNotification4; break;
						case 5: icon = this->IconNotification5; break;
						case 6: icon = this->IconNotification6; break;
						case 7: icon = this->IconNotification7; break;
						case 8: icon = this->IconNotification8; break;
						case 9: icon = this->IconNotification9; break;
						default: icon = this->IconNotificationTooMany; break;
					}

					float offIcon = (size * UIRoot::ScalingFactor) / 2.0f;

					pos.x += offIcon;
					pos.y += offIcon;

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::SetCursorPos(pos);
					ImGui::Image(icon->Resource, ImVec2(offIcon, offIcon));
					ImGui::PopItemFlag();
					notifHovered = ImGui::IsItemHovered();
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
						ImGui::Text("%s (%s)", this->Language->Translate(shortcut.TooltipText.c_str()), shortcut.IBText.c_str());
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
					ImGui::TextDisabled(loader->GetOwner(cbshortcut.Callback).c_str());
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
	std::string str = aIdentifier;
	std::string strTexId = aTextureIdentifier;
	std::string strTexHoverId = aTextureHoverIdentifier;
	std::string strKbId = aInputBindIdentifier;
	std::string strTT = aTooltipText;

	/* explicit scoping due to this->WhereAreMyParents() below */
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		if (this->Registry.find(str) == this->Registry.end())
		{
			Texture_t* normal = this->TextureService->Get(strTexId.c_str());
			Texture_t* hover = this->TextureService->Get(strTexHoverId.c_str());
			Shortcut_t sh{};
			sh.TextureNormalIdentifier = aTextureIdentifier;
			sh.TextureHoverIdentifier = aTextureHoverIdentifier;
			sh.TextureNormal = normal;
			sh.TextureHover = hover;
			sh.IBIdentifier = aInputBindIdentifier;

			const InputBind_t* ib = this->InputBindApi->Get(aInputBindIdentifier);

			assert(ib != nullptr);

			if (ib->Device != EInputDevice::None)
			{
				sh.IBText = IBToString(*ib, true);
			}

			sh.TooltipText = aTooltipText;
			sh.TextureGetAttempts = 0;
			this->Registry[str] = sh;

			int amt = 0;
			if (sh.TextureNormal != nullptr) { amt++; }
			if (sh.TextureHover != nullptr) { amt++; }
		}
	}

	this->WhereAreMyParents();

	this->Validate(true);
}

void CQuickAccess::RemoveShortcut(const char* aIdentifier)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(str);

	if (it != this->Registry.end())
	{
		for (auto& orphan : it->second.ContextItems)
		{
			this->OrphanedCallbacks[orphan.first] = orphan.second;
		}
	}

	this->Registry.erase(str);

	this->Validate(false);
}

void CQuickAccess::NotifyShortcut(const char* aIdentifier, const char* aNotificationKey)
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

void CQuickAccess::DenotifyShortcut(const char* aIdentifier, const char* aNotificationKey)
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
	this->AddContextItem(aIdentifier, QA_MENU, aShortcutRenderCallback);
}

void CQuickAccess::AddContextItem(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback)
{
	std::string str = aIdentifier;
	std::string tarShStr = aTargetShortcutIdentifier ? aTargetShortcutIdentifier : QA_MENU;

	ContextItem_t shortcut{
		aIdentifier,
		aShortcutRenderCallback
	};

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(tarShStr);
	if (it != this->Registry.end())
	{
		it->second.ContextItems[str] = shortcut;
	}
	else
	{
		this->OrphanedCallbacks[str] = shortcut;
	}

	this->Validate(false);
}

void CQuickAccess::RemoveContextItem(const char* aIdentifier)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, shortcut] : this->Registry)
	{
		shortcut.ContextItems.erase(aIdentifier);
	}

	this->OrphanedCallbacks.erase(aIdentifier);

	this->Validate(false);
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

int CQuickAccess::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* remove active callbacks */
	for (auto& [identifier, shortcut] : this->Registry)
	{
		std::vector<std::string> remove;

		for (auto& [cbidentifier, cbshortcut] : shortcut.ContextItems)
		{
			if (cbshortcut.Callback >= aStartAddress && cbshortcut.Callback <= aEndAddress)
			{
				remove.push_back(cbidentifier);
				refCounter++;
			}
		}
	}

	/* remove bastard children */
	std::vector<std::string> remove;

	for (auto& [cbidentifier, cbshortcut] : this->OrphanedCallbacks)
	{
		if (cbshortcut.Callback >= aStartAddress && cbshortcut.Callback <= aEndAddress)
		{
			remove.push_back(cbidentifier);
			refCounter++;
		}
	}

	for (std::string remidentifier : remove)
	{
		this->OrphanedCallbacks.erase(remidentifier);
	}

	this->Validate(false);

	return refCounter;
}

void CQuickAccess::Validate(bool aLock)
{
	if (aLock)
	{
		this->Mutex.lock();
	}

	int amtValid = 0;

	for (auto& [identifier, shortcut] : this->Registry)
	{
		shortcut.IsValid = this->IsValid(shortcut);

		if (this->ShowArcDPSShortcut == false && identifier == QA_ARCDPS)
		{
			shortcut.IsValid = false;
		}

		if (shortcut.IsValid) { amtValid++; }
	}

	if (aLock)
	{
		this->Mutex.unlock();
	}

	this->NexusLink->QuickAccessIconsCount = amtValid;
}

void CQuickAccess::WhereAreMyParents()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	std::vector<std::string> remove;

	for (auto& [identifier, shortcut] : this->OrphanedCallbacks)
	{
		auto it = this->Registry.find(shortcut.TargetShortcut);
		if (it != this->Registry.end())
		{
			it->second.ContextItems[identifier] = shortcut;
		}
	}

	/* no longer orphans */
	for (std::string remidentifier : remove)
	{
		this->OrphanedCallbacks.erase(remidentifier);
	}
}

bool CQuickAccess::IsValid(const Shortcut_t& aShortcut)
{
	return aShortcut.ContextItems.size() > 0 || this->InputBindApi->HasHandler(aShortcut.IBIdentifier);
}

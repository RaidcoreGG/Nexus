///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QuickAccess.cpp
/// Description  :  Contains the logic for the Quick Access HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "QuickAccess.h"

#include "ImAnimate/ImAnimate.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "Core/Addons/AddConst.h"
#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"
#include "Core/Preferences/PrefContext.h"
#include "Engine/Cleanup/RefCleanerContext.h"
#include "Resources/ResConst.h"
#include "UI/UIContext.h"
#include "Util/Time.h"

#define GW2_QUICKACCESS_ITEMS 10;

void CQuickAccess::OnAddonStateChanged(void* aEventData)
{
	CContext*     ctx   = CContext::GetContext();
	CUiContext*   uictx = ctx->GetUIContext();
	CQuickAccess* qactx = uictx->GetQuickAccess();

	qactx->Invalidate();
}

CQuickAccess::CQuickAccess(CDataLinkApi* aDataLink, CLogApi* aLogger, CInputBindApi* aInputBindApi, CTextureLoader* aTextureService, CLocalization* aLocalization, CEventApi* aEventApi)
{
	this->Logger         = aLogger;
	this->InputBindApi   = aInputBindApi;
	this->TextureService = aTextureService;
	this->Language       = aLocalization;
	this->EventApi       = aEventApi;

	this->NexusLink = (NexusLinkData_t*)aDataLink->GetResource(DL_NEXUS_LINK);
	this->MumbleLink = (Mumble::Data*)aDataLink->GetResource(DL_MUMBLE_LINK);

	CContext*  ctx         = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	/* Setup notifiers. */
	settingsctx->Subscribe<bool>(OPT_QAVERTICAL, [&](bool aVertical)
	{
		this->VerticalLayout = aVertical;
	});
	settingsctx->Subscribe<EQaPosition>(OPT_QALOCATION, [&](EQaPosition aPosition)
	{
		this->Location = aPosition;
	});
	settingsctx->Subscribe<float>(OPT_QAOFFSETX, [&](float aOffsetX)
	{
		this->Offset.x = aOffsetX;
	});
	settingsctx->Subscribe<float>(OPT_QAOFFSETY, [&](float aOffsetY)
	{
		this->Offset.y = aOffsetY;
	});
	settingsctx->Subscribe<EQaVisibility>(OPT_QAVISIBILITY, [&](EQaVisibility aVisibility)
	{
		this->Visibility = aVisibility;
	});
	settingsctx->Subscribe<bool>(OPT_QAONLYSHOWONHOVER, [&](bool aOnlyShowOnHover)
	{
		this->OnlyShowOnHover = aOnlyShowOnHover;
	});
	settingsctx->Subscribe<std::vector<std::string>>(OPT_QASUPPRESSED, [&](std::vector<std::string> aSuppressedShortcuts)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->SuppressedShortcuts = aSuppressedShortcuts;

		for (auto& [id, shortcut] : this->Registry)
		{
			/* If the current shortcut ID is in the suppressed list. */
			bool isSuppressed = std::find(this->SuppressedShortcuts.begin(), this->SuppressedShortcuts.end(), id) != this->SuppressedShortcuts.end();

			shortcut->SetSuppression(isSuppressed);
		}
	});

	/* Preload default icons. */
	this->TextureService->Load(ICON_NEXUS,                 RES_ICON_NEXUS,                 ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_HOVER,           RES_ICON_NEXUS_HOVER,           ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_HALLOWEEN,       RES_ICON_NEXUS_HALLOWEEN,       ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_HALLOWEEN_HOVER, RES_ICON_NEXUS_HALLOWEEN_HOVER, ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_XMAS,            RES_ICON_NEXUS_XMAS,            ctx->GetModule(), nullptr);
	this->TextureService->Load(ICON_NEXUS_XMAS_HOVER,      RES_ICON_NEXUS_XMAS_HOVER,      ctx->GetModule(), nullptr);

	/// FIXME: This is kinda hacky.
	/// It forces the creation of the setting, so that the below subscriber gets executed on a first launch.
	/// Otherwise the main menu would need to be opened first (static init of snowflakemgr) or the options in the main menu (static init of setting).
	/// Both is kinda shit.
	settingsctx->Get<bool>(OPT_DISABLEFESTIVEFLAIR, false);

	settingsctx->Subscribe<bool>(OPT_DISABLEFESTIVEFLAIR, [&](bool aDisableFestiveFlair)
	{
		/* Remove existing shortcut. */
		this->RemoveShortcut(QA_MENU);

		const char* icon      = ICON_NEXUS;
		const char* iconHover = ICON_NEXUS_HOVER;

		if (!aDisableFestiveFlair)
		{
			switch (Time::GetMonth())
			{
				case 10:
				{
					icon = ICON_NEXUS_HALLOWEEN;
					iconHover = ICON_NEXUS_HALLOWEEN_HOVER;
					break;
				}
				case 12:
				{
					icon = ICON_NEXUS_XMAS;
					iconHover = ICON_NEXUS_XMAS_HOVER;
					break;
				}
			}
		}

		/* Recreate shortcut with appropriate icon. */
		this->AddShortcut(QA_MENU, icon, iconHover, KB_MENU, "((000009))");
	});

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
	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (this->IsInvalid)
	{
		this->UpdateNexusLink();
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

	uint32_t isActive = 0;

	ImVec2 wndPos = ImVec2(0.0f, 0.0f);

	bool isHoveringNative = false;
	static float s_IconBaseSize = 32.f;
	switch (this->Location)
	{
		case EQaPosition::Extend:
		{
			wndPos.x += (s_IconBaseSize * UIRoot::ScalingFactor) * GW2_QUICKACCESS_ITEMS;

			ImVec2 mPos = ImGui::GetMousePos();
			if (mPos.x != -FLT_MAX && mPos.y != -FLT_MAX && mPos.x < wndPos.x - this->Offset.x && mPos.y < UIRoot::ScalingFactor * s_IconBaseSize)
			{
				isHoveringNative = true;
			}
			break;
		}
		case EQaPosition::Under:
		{
			wndPos.y += s_IconBaseSize * UIRoot::ScalingFactor;
			break;
		}
		case EQaPosition::Bottom:
		{
			CContext*        ctx      = CContext::GetContext();
			RenderContext_t* renderer = ctx->GetRendererCtx();
			wndPos.y += renderer->Window.Height - (s_IconBaseSize * 2 * UIRoot::ScalingFactor);
			break;
		}
	}

	wndPos.x += this->Offset.x;
	wndPos.y += this->Offset.y;

	ImGuiWindowFlags flags
		= ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, this->Opacity);

	bool poppedStyles = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::SetNextWindowPos(wndPos);
	if (ImGui::Begin("QuickAccess", 0, flags))
	{
		poppedStyles = true;
		ImGui::PopStyleVar(1); // window padding

		for (auto& [identifier, shortcut] : this->Registry)
		{
			isActive += shortcut->Render();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.f, 1.f) * UIRoot::ScalingFactor);
			if (!this->VerticalLayout)
			{
				/* If not vertical (aka is horizontal) force SameLine(). */
				ImGui::SameLine();
			}
			else
			{
				/// If vertical, force SameLine(), then undo it with NewLine().
				/// NewLine() alone or no NewLine() at all unsets with the spacing.
				/// #justimguithings
				ImGui::SameLine();
				ImGui::NewLine();
			}
			ImGui::PopStyleVar(1);
		}

		bool windowHovered = ImGui::IsWindowHovered();
		bool isHovering = windowHovered || isActive || isHoveringNative;

		float opacityMin = 0.5f;
		float opacityMax = 1.0f;

		if (this->OnlyShowOnHover)
		{
			opacityMin = 0.001f;
		}

		if (isHovering)
		{
			ImGui::Animate(opacityMin, opacityMax, 350, &this->Opacity, ImAnimate::ECurve::Linear);
		}
		else
		{
			ImGui::Animate(opacityMax, opacityMin, 350, &this->Opacity, ImAnimate::ECurve::Linear);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(); // alpha

	if (!poppedStyles)
	{
		ImGui::PopStyleVar(1); // window padding
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
		CShortcutIcon* shortcut = new CShortcutIcon(
			aIdentifier,
			aTextureIdentifier,
			aTextureHoverIdentifier,
			aInputBindIdentifier ? aInputBindIdentifier : "",
			aTooltipText ? aTooltipText : ""
		);

		CContext*  ctx         = CContext::GetContext();
		CSettings* settingsctx = ctx->GetSettingsCtx();

		/* If the current shortcut ID is in the suppressed list. */
		bool isSuppressed = std::find(this->SuppressedShortcuts.begin(), this->SuppressedShortcuts.end(), aIdentifier) != this->SuppressedShortcuts.end();

		shortcut->SetSuppression(isSuppressed);

		this->Registry.emplace(aIdentifier, shortcut);
	}

	/* Check if this shortcut can maybe adopt some orphans. */
	this->WhereAreMyParents();
	this->Invalidate();
}

void CQuickAccess::RemoveShortcut(const char* aIdentifier)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		for (auto& [identifier, ctxitem] : it->second->GetContextMenuItems())
		{
			auto orphanIt = this->OrphanedCallbacks.find(identifier);

			if (orphanIt == this->OrphanedCallbacks.end())
			{
				this->OrphanedCallbacks.emplace(identifier, ctxitem);
			}
		}

		delete it->second;
		this->Registry.erase(it);
	}

	this->Invalidate();
}

void CQuickAccess::PushNotification(const char* aIdentifier, const char* aNotificationKey)
{
	if (!aIdentifier)      { return; }
	if (!aNotificationKey) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		it->second->PushNotifcationSafe(aNotificationKey);
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
		it->second->PopNotifcationSafe(aNotificationKey);
	}
}

void CQuickAccess::AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback)
{
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
		parentIt->second->AddContextItem(aIdentifier, contextItem);
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

	this->Invalidate();
}

void CQuickAccess::RemoveContextItem(const char* aIdentifier)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, shortcut] : this->Registry)
	{
		shortcut->RemoveContextItem(aIdentifier);
	}

	this->OrphanedCallbacks.erase(aIdentifier);

	this->Invalidate();
}

std::map<std::string, CShortcutIcon*> CQuickAccess::GetRegistry() const
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
		refCounter += shortcut->CleanupRefs(aStartAddress, aEndAddress);
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

	this->Invalidate();

	return refCounter;
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
			parentIt->second->AddContextItem(orphanIt->first, orphanIt->second);

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

void CQuickAccess::UpdateNexusLink()
{
	int amtValid = 0;

	for (auto& [identifier, shortcut] : this->Registry)
	{
		if (shortcut->IsActive())
		{
			amtValid++;
		}
	}

	this->NexusLink->QuickAccessIconsCount = amtValid;
}

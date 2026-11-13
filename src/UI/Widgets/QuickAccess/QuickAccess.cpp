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

#include "Host/GW2/Addons/AddConst.h"
#include "Host/GW2/Context.h"
#include "UI/Widgets/QuickAccess/QaConst.h"
#include "res/Resources.h"
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

CQuickAccess::CQuickAccess(
	CDataLinkApi*   aDataLink,
	CInputBindApi*  aInputBindApi,
	CTextureLoader* aTextureService,
	CLocalization*  aLocalization,
	CEventApi*      aEventApi
) : IRefCleaner("QuickAccess")
{
	this->InputBindApi   = aInputBindApi;
	this->TextureService = aTextureService;
	this->Language       = aLocalization;
	this->EventApi       = aEventApi;

	this->NexusLink = (NexusLinkData_t*)aDataLink->GetResource(DL_NEXUS_LINK);
	this->MumbleLink = (Mumble::Data*)aDataLink->GetResource(DL_MUMBLE_LINK);

	/* Preload default icons. */
	this->TextureService->Load(ICON_NEXUS,                 Resources::IconNexus, nullptr);
	this->TextureService->Load(ICON_NEXUS_HOVER,           Resources::IconNexusHover, nullptr);
	this->TextureService->Load(ICON_NEXUS_HALLOWEEN,       Resources::IconNexusHalloween, nullptr);
	this->TextureService->Load(ICON_NEXUS_HALLOWEEN_HOVER, Resources::IconNexusHalloweenHover, nullptr);
	this->TextureService->Load(ICON_NEXUS_XMAS,            Resources::IconNexusXmas, nullptr);
	this->TextureService->Load(ICON_NEXUS_XMAS_HOVER,      Resources::IconNexusXmasHover, nullptr);

	this->UpdateMenuIcon(this->DisableFestiveFlair.Get());

	this->EventApi->Subscribe(EV_ADDON_LOADED,   CQuickAccess::OnAddonStateChanged);
	this->EventApi->Subscribe(EV_ADDON_UNLOADED, CQuickAccess::OnAddonStateChanged);
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

	this->NexusLink->QuickAccessMode = (int)this->Location.Get();
	this->NexusLink->QuickAccessIsVertical = this->VerticalLayout.Get();

	switch (this->Visibility.Get())
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
	switch (this->Location.Get())
	{
		case EQaPosition::Extend:
		{
			wndPos.x += (s_IconBaseSize * this->NexusLink->Scaling) * GW2_QUICKACCESS_ITEMS;
			ImVec2 mPos = ImGui::GetMousePos();
			if (mPos.x != -FLT_MAX && mPos.y != -FLT_MAX && mPos.x < wndPos.x - this->OffsetX.Get() && mPos.y < this->NexusLink->Scaling * s_IconBaseSize)
			{
				isHoveringNative = true;
			}
			break;
		}
		case EQaPosition::Under:
		{
			wndPos.y += s_IconBaseSize * this->NexusLink->Scaling;
			break;
		}
		case EQaPosition::Bottom:
		{
			CContext*        ctx      = CContext::GetContext();
			RenderContext_t* renderer = ctx->GetRendererCtx();
			wndPos.y += renderer->Window.Height - (s_IconBaseSize * 2 * this->NexusLink->Scaling);
			break;
		}
	}

	wndPos.x += this->OffsetX.Get();
	wndPos.y += this->OffsetY.Get();

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

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.f, 1.f) * this->NexusLink->Scaling);
			if (!this->VerticalLayout.Get())
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

		if (this->OnlyShowOnHover.Get())
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

		/* If the current shortcut ID is in the suppressed list. */
		const std::vector<std::string> suppressed = this->SuppressedShortcuts.Get();
		bool isSuppressed = std::find(suppressed.begin(), suppressed.end(), aIdentifier) != suppressed.end();

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
			LogWarning(
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

uint32_t CQuickAccess::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

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

void CQuickAccess::UpdateMenuIcon(bool aDisableFestiveFlair)
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
	this->AddShortcut(QA_MENU, icon, iconHover, KB_MENU, Lang::QuickAccess::NexusMenu.Key());
}

void CQuickAccess::ApplySuppression(const std::vector<std::string>& aSuppressedShortcuts)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [id, shortcut] : this->Registry)
	{
		/* If the current shortcut ID is in the suppressed list. */
		bool isSuppressed = std::find(aSuppressedShortcuts.begin(), aSuppressedShortcuts.end(), id) != aSuppressedShortcuts.end();

		shortcut->SetSuppression(isSuppressed);
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

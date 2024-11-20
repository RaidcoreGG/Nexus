///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Debug.cpp
/// Description  :  Contains the content of the debug window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Debug.h"

#include <unordered_map>

#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "Consts.h"
#include "Context.h"
#include "Events/EventHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "resource.h"
#include "Util/MD5.h"
#include "Util/Strings.h"

static CDebugWindow* DebugWindow = nullptr;
static void DebugWindow_OnInputBind(const char* aIdentifier)
{
	if (DebugWindow)
	{
		DebugWindow->ToggleMumbleOverlay();
	}
}

CDebugWindow::CDebugWindow()
{
	this->Name           = "Debug";
	this->DisplayName    = "((000007))";
	this->IconIdentifier = "ICON_DEBUG";
	this->IconID         = RES_ICON_DEBUG;
	this->IsHost         = true; /* set to true to enable RenderSubWindows() call*/

	/* bind mumble overlay */
	CContext* ctx = CContext::GetContext();
	ctx->GetInputBindApi()->Register(KB_MUMBLEOVERLAY, EInputBindHandlerType::DownOnly, DebugWindow_OnInputBind, NULLSTR);
	DebugWindow = this;
}

void CDebugWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CEscapeClosing* escclose = uictx->GetEscapeClosingService();

		escclose->Deregister(this->GetVisibleStatePtr());
		escclose->Register(this->GetNameID().c_str(), this->GetVisibleStatePtr());

		this->IsInvalid = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
	ImGui::Checkbox("Show Mumble overlay", this->MumbleWindow->GetVisibleStatePtr());
	ImGui::Checkbox("Show ImGui Debugger", &this->IsMetricsWindowVisible);
	ImGui::PopStyleVar();

	if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None))
	{
		this->TabEvents();
		this->TabInputBinds();
		this->TabDataLink();
		this->TabTextures();
		this->TabQuickAccess();
		this->TabLoader();
		this->TabFonts();
		ImGui::EndTabBar();
	}
}

void CDebugWindow::RenderSubWindows()
{
	this->MemoryViewer.DrawWindow("Memory Viewer", this->MV_Ptr, this->MV_Size);

	if (this->IsMetricsWindowVisible)
	{
		ImGui::ShowMetricsWindow();
	}

	this->MumbleWindow->Render();
}

void CDebugWindow::ToggleMumbleOverlay()
{
	bool* visible = this->MumbleWindow->GetVisibleStatePtr();
	*visible = !*visible;
}

void CDebugWindow::TabEvents()
{
	if (!ImGui::BeginTabItem("Events"))
	{
		return;
	}

	if (ImGui::BeginChild("EventsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		std::unordered_map<std::string, EventData> eventRegistry = CContext::GetContext()->GetEventApi()->GetRegistry();

		for (auto& [identifier, ev] : eventRegistry)
		{
			if (ImGui::TreeNode(String::Format("%s (%d)", identifier.c_str(), ev.AmountRaises).c_str()))
			{
				if (ev.Subscribers.size() == 0)
				{
					ImGui::TextDisabled("This event has no subscribers.");
				}
				else
				{
					ImGui::TextDisabled("Subscribers:");
					for (EventSubscriber sub : ev.Subscribers)
					{
						ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabled("Signature: %d | Callback: %p", sub.Signature, sub.Callback);
					}
				}
				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabInputBinds()
{
	if (!ImGui::BeginTabItem("InputBinds"))
	{
		return;
	}

	if (ImGui::BeginChild("InputBindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		std::map<std::string, ManagedInputBind> inputBindRegistry = CContext::GetContext()->GetInputBindApi()->GetRegistry();

		for (auto& [identifier, inputBind] : inputBindRegistry)
		{
			ImGui::Text(identifier.c_str()); ImGui::SameLine();
			if (inputBind.Handler)
			{
				ImGui::TextDisabled("Handler: %p", inputBind.Handler);
			}
			else
			{
				ImGui::TextDisabled("Handler: (null)");
			}
		}
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabDataLink()
{
	if (!ImGui::BeginTabItem("DataLink"))
	{
		return;
	}

	if (ImGui::BeginChild("DataLinkTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		std::unordered_map<std::string, LinkedResource>	dataLinkRegistry = CContext::GetContext()->GetDataLink()->GetRegistry();

		for (auto& [identifier, resource] : dataLinkRegistry)
		{
			if (ImGui::TreeNode(identifier.c_str()))
			{
				ImGui::TextDisabled("Handle: %p", resource.Handle);
				ImGui::TextDisabled("Pointer: %p", resource.Pointer);
				ImGui::TextDisabled("Size: %d", resource.Size);
				ImGui::TextDisabled("Name: %s", resource.UnderlyingName.c_str());
				ImGui::TooltipGeneric("The real underlying name of the file.");

				if (ImGui::SmallButton("Memory Viewer"))
				{
					this->MemoryViewer.Open = true;
					this->MV_Ptr = resource.Pointer;
					this->MV_Size = resource.Size;
				}

				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabTextures()
{
	if (!ImGui::BeginTabItem("Textures"))
	{
		return;
	}

	if (ImGui::BeginChild("TexturesTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		std::map<std::string, Texture*> texRegistry = CContext::GetContext()->GetTextureService()->GetRegistry();
		std::vector<QueuedTexture>      texQueued = CContext::GetContext()->GetTextureService()->GetQueuedTextures();

		float previewSize = ImGui::GetTextLineHeightWithSpacing() * 3;
		float wndHeight = ImGui::GetWindowHeight();

		static char texFilter[400] = {};
		static int displayedTextures = 0;
		static size_t displayedMemUsage = 0;

		ImGui::Text("Filter:");
		ImGui::SameLine();
		ImGui::InputText("##Filter", &texFilter[0], 400);

		std::string texFilterStr = texFilter;

		ImGui::Text("Displaying %d of %d loaded textures:", displayedTextures, texRegistry.size());
		ImGui::Text("Combined memory usage of displayed: %s", String::FormatByteSize(displayedMemUsage).c_str());

		ImVec2 drawPos = ImGui::GetCursorPos();

		ImGuiStyle& style = ImGui::GetStyle();

		int amt = 0;
		size_t mem = 0;

		for (auto& [identifier, texture] : texRegistry)
		{
			if (!texFilterStr.empty() && !String::Contains(identifier, texFilterStr))
			{
				continue;
			}

			amt++;

			/* query texture size */
			{
				ID3D11Resource* pResource = nullptr;
				texture->Resource->GetResource(&pResource);

				ID3D11Texture2D* pTexture2D = nullptr;
				if (SUCCEEDED(pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture2D)))
				{
					D3D11_TEXTURE2D_DESC desc;
					pTexture2D->GetDesc(&desc);

					UINT64 totalBytes = 0;
					UINT64 width = desc.Width;
					UINT64 height = desc.Height;

					UINT64 formatSize = 4; // 4 bytes (DXGI_FORMAT_R8G8B8A8_UNORM)

					for (UINT mip = 0; mip < desc.MipLevels; mip++)
					{
						UINT64 mipSize = width * height * formatSize;
						totalBytes += mipSize;

						// Halve the dimensions for the next mip level
						width = (width > 1) ? (width / 2) : 1;
						height = (height > 1) ? (height / 2) : 1;
					}

					mem += totalBytes;

					pTexture2D->Release();
				}
				pResource->Release();
			}

			/*  above visible space                           || under visible space */
			if (drawPos.y < ImGui::GetScrollY() - previewSize || drawPos.y > ImGui::GetScrollY() + wndHeight)
			{
				ImGui::SetCursorPos(drawPos);
				ImGui::Dummy(ImVec2(previewSize, previewSize));
				drawPos.y += previewSize + style.ItemSpacing.y;
				continue;
			}

			ImGui::SetCursorPos(drawPos);
			ImGui::Image(texture->Resource, ImVec2(previewSize, previewSize));

			if (ImGui::IsItemHovered())
			{
				if (ImGui::Tooltip())
				{
					if (texture->Resource)
					{
						float scale = 1.0f;
						if (texture->Width > 400.0f || texture->Height > 400.0f)
						{
							scale = (texture->Width > texture->Height ? texture->Width : texture->Height) / 400.0f;
						}
						float previewWidth = texture->Width / scale;
						float previewHeight = texture->Height / scale;

						ImGui::Image(texture->Resource, ImVec2(previewWidth, previewHeight));
					}

					ImGui::EndTooltip();
				}
			}

			float xOffsetDetails = drawPos.x + previewSize + style.ItemSpacing.x;

			ImGui::SetCursorPos(ImVec2(xOffsetDetails, drawPos.y));
			ImGui::Text("Identifier: %s", identifier.c_str());

			ImGui::SetCursorPos(ImVec2(xOffsetDetails, drawPos.y + ImGui::GetTextLineHeightWithSpacing()));
			ImGui::TextDisabled("Dimensions: %dx%d", texture->Width, texture->Height);

			ImGui::SetCursorPos(ImVec2(xOffsetDetails, drawPos.y + ImGui::GetTextLineHeightWithSpacing() * 2));
			ImGui::TextDisabled("Pointer: %p", texture->Resource);

			drawPos.y += previewSize + style.ItemSpacing.y;
		}

		displayedTextures = amt;
		displayedMemUsage = mem;

		if (texRegistry.size() == 0)
		{
			ImGui::TextDisabled("No textures loaded.");
		}
		ImGui::Separator();
		ImGui::Text("Queued Textures:");
		for (auto& qtexture : texQueued)
		{
			if (ImGui::TreeNode(qtexture.Identifier.c_str()))
			{
				ImGui::TextDisabled("Dimensions: %dx%d", qtexture.Width, qtexture.Height);
				ImGui::TextDisabled("ReceiveCallback: %p", qtexture.Callback);
				ImGui::TextDisabled("Data: ", qtexture.Data);

				ImGui::TreePop();
			}
		}
		if (texQueued.size() == 0)
		{
			ImGui::TextDisabled("No textures queued for loading.");
		}

		texRegistry.clear();
		texQueued.clear();
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabQuickAccess()
{
	if (!ImGui::BeginTabItem("Quick Access"))
	{
		return;
	}

	if (ImGui::BeginChild("QuickAccessTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		ImGui::Text("Items:");

		std::map<std::string, Shortcut> qaRegistry = CContext::GetContext()->GetUIContext()->GetQuickAccess()->GetRegistry();

		for (auto& [identifier, shortcut] : qaRegistry)
		{
			if (ImGui::TreeNode(identifier.c_str()))
			{
				ImGui::TextDisabled("Texture: %p", shortcut.TextureNormal != nullptr ? shortcut.TextureNormal->Resource : nullptr);
				ImGui::TextDisabled("Texture (Hover): %p", shortcut.TextureHover != nullptr ? shortcut.TextureHover->Resource : nullptr);
				ImGui::TextDisabled("OnClick (InputBind): %s", shortcut.IBIdentifier.length() != 0 ? shortcut.IBIdentifier.c_str() : NULLSTR);
				ImGui::TextDisabled("Tooltip: %s", shortcut.TooltipText.length() != 0 ? shortcut.TooltipText.c_str() : NULLSTR);
				ImGui::TextDisabled("IsHovering: %s", shortcut.IsHovering ? "true" : "false");
				if (shortcut.ContextItems.size() > 0)
				{
					ImGui::Text("Context Menu Items:");
					for (auto& [identifier, shortcut] : shortcut.ContextItems)
					{
						if (ImGui::TreeNode(identifier.c_str()))
						{
							ImGui::TextDisabled("Callback: %p", shortcut.Callback);

							ImGui::TreePop();
						}
					}
				}
				ImGui::TreePop();
			}
		}

		ImGui::Separator();

		std::map<std::string, ContextItem> orphanage = CContext::GetContext()->GetUIContext()->GetQuickAccess()->GetOrphanage();

		ImGui::Text("Orphaned context menu items:");
		for (auto& [identifier, shortcut] : orphanage)
		{
			if (ImGui::TreeNode(identifier.c_str()))
			{
				ImGui::TextDisabled("Callback: %p", shortcut);

				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabLoader()
{
	if (!ImGui::BeginTabItem("Loader"))
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(Loader::Mutex);
	
	if (ImGui::BeginChild("LoaderTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		if (ImGui::TreeNode("Tracked"))
		{
			for (auto addon : Loader::Addons)
			{
				std::string cached = "Currently not installed: ";
				if (ImGui::TreeNode(addon->Path.empty() ? (cached + std::to_string(addon->MatchSignature)).c_str() : addon->Path.string().c_str()))
				{
					ImGui::Text("MatchSignature: %d", addon->MatchSignature);

					std::string state = "State: ";
					switch (addon->State)
					{
						case EAddonState::None:                     state.append("None"); break;
						case EAddonState::Loaded:                   state.append("Loaded"); break;
						case EAddonState::LoadedLOCKED:             state.append("LoadedLOCKED"); break;
						case EAddonState::NotLoaded:                state.append("NotLoaded"); break;
						case EAddonState::NotLoadedDuplicate:       state.append("NotLoadedDuplicate"); break;
						case EAddonState::NotLoadedIncompatible:    state.append("NotLoadedIncompatible"); break;
						case EAddonState::NotLoadedIncompatibleAPI: state.append("NotLoadedIncompatibleAPI"); break;
					}

					ImGui::TextDisabled(state.c_str());
					ImGui::TextDisabled("Module: %p", addon->Module);
					ImGui::TextDisabled("Module Size: %u", addon->ModuleSize);
					ImGui::TextDisabled("MD5: %s", MD5Util::ToString(addon->MD5).c_str());
					ImGui::TextDisabled("Definitions: %p", addon->Definitions);
					ImGui::Separator();
					ImGui::TextDisabled("IsFlaggedForDisable: %s", addon->IsFlaggedForDisable ? "true" : "false");
					ImGui::TextDisabled("IsPausingUpdates: %s", addon->IsPausingUpdates ? "true" : "false");
					ImGui::TextDisabled("AllowPrereleases: %s", addon->AllowPrereleases ? "true" : "false");
					ImGui::TextDisabled("IsFavorite: %s", addon->IsFavorite ? "true" : "false");
					ImGui::TextDisabled("IsFlaggedForUninstall: %s", addon->IsFlaggedForUninstall ? "true" : "false");
					ImGui::TextDisabled("IsDisabledUntilUpdate: %s", addon->IsDisabledUntilUpdate ? "true" : "false");

					if (addon->Definitions != nullptr)
					{
						if (ImGui::SmallButton("Memory Viewer"))
						{
							this->MemoryViewer.Open = true;
							this->MV_Ptr = addon->Definitions;
							this->MV_Size = sizeof(AddonDefinition);
						}
					}

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Queued"))
		{
			for (const auto& [path, action] : Loader::QueuedAddons)
			{
				switch (action)
				{
					case ELoaderAction::Load:
						ImGui::Text("Load");
						break;
					case ELoaderAction::Unload:
						ImGui::Text("Unload");
						break;
					case ELoaderAction::Uninstall:
						ImGui::Text("Uninstall");
						break;
				}
				ImGui::SameLine();
				ImGui::TextDisabled("%s", path.string().c_str());
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

void CDebugWindow::TabFonts()
{
	if (!ImGui::BeginTabItem("Fonts"))
	{
		return;
	}

	if (ImGui::BeginChild("FontsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFontAtlas* atlas = io.Fonts;

		ImGui::PushItemWidth(120);
		for (int i = 0; i < atlas->Fonts.Size; i++)
		{
			ImFont* font = atlas->Fonts[i];
			ImGui::PushID(font);
			ImGui::NodeFont(font);
			ImGui::PopID();
		}

		if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
		{
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
			ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
			ImGui::TreePop();
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();

	ImGui::EndTabItem();
}

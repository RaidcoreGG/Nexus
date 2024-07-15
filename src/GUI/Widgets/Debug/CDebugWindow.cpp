#include "CDebugWindow.h"

#include "Shared.h"
#include "State.h"

#include "Events/EventHandler.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"
#include "Inputs/Keybinds/KeybindHandler.h"
#include "Loader/Loader.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Textures/TextureLoader.h"

#include "Util/MD5.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "imgui/imgui_memory_editor.h"

namespace GUI
{
	float dwWidth = 30.0f;
	float dwHeight = 24.0f;

	static ImGui::MemoryEditor memEditor;
	void* memPtr = nullptr;
	size_t memSz = 0;

	bool showMetricsDebugger = false;

	/* proto tabs */
	void DbgEventsTab();
	void DbgKeybindsTab();
	void DbgDataLinkTab();
	void DbgTexturesTab();
	void DbgShortcutsTab();
	void DbgLoaderTab();
	void DbgAPITab();
	void DbgFontsTab();

	void NodeFont(ImFont* font);

	CDebugWindow::CDebugWindow(std::string aName)
	{
		Name = aName;
		MumbleWindow = new CMumbleOverlay();
	}

	void CDebugWindow::Render()
	{
		MumbleWindow->Render();

		memEditor.DrawWindow("Memory Editor", memPtr, memSz);

		if (!Visible) { return; }

		if (showMetricsDebugger)
		{
			ImGui::ShowMetricsWindow();
		}

		ImGui::SetNextWindowSize(ImVec2(dwWidth * ImGui::GetFontSize(), dwHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
			ImGui::Checkbox("Show Mumble overlay", &MumbleWindow->Visible);
			ImGui::Checkbox("Show Metrics / Debugger", &showMetricsDebugger);
			ImGui::PopStyleVar();

			if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None))
			{
				DbgEventsTab();
				DbgKeybindsTab();
				DbgDataLinkTab();
				DbgTexturesTab();
				DbgShortcutsTab();
				DbgLoaderTab();
				DbgAPITab();
				DbgFontsTab();
				/*if (ImGui::BeginTabItem("Meme"))
				{
					{
						ImGui::BeginChild("##MemeTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						// new item

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}*/
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void DbgEventsTab()
	{
		if (ImGui::BeginTabItem("Events"))
		{
			ImGui::BeginChild("##EventsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			std::unordered_map<std::string, EventData> EventRegistry = EventApi->GetRegistry();

			for (auto& [identifier, ev] : EventRegistry)
			{
				std::string header = identifier + " ";
				header.append("(");
				header.append(std::to_string(ev.AmountRaises));
				header.append(")");

				if (ImGui::TreeNode(header.c_str()))
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
							ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabled("Owner: %d | Callback: %p", sub.Signature, sub.Callback);
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}
	void DbgKeybindsTab()
	{
		if (ImGui::BeginTabItem("Keybinds"))
		{
			ImGui::BeginChild("##KeybindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			std::map<std::string, ActiveKeybind> KeybindRegistry = KeybindApi->GetRegistry();
			
			for (auto& [identifier, keybind] : KeybindRegistry)
			{
				ImGui::Text(identifier.c_str()); ImGui::SameLine();
				if (keybind.Handler)
				{
					ImGui::TextDisabled("Handler: %p", keybind.Handler);
				}
				else
				{
					ImGui::TextDisabled("Handler: (null)");
				}
			}

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}
	void DbgDataLinkTab()
	{
		if (ImGui::BeginTabItem("DataLink"))
		{
			ImGui::BeginChild("##DataLinkTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			std::unordered_map<std::string, LinkedResource>	DataLinkRegistry = DataLinkService->GetRegistry();

			for (auto& [identifier, resource] : DataLinkRegistry)
			{
				if (ImGui::TreeNode(identifier.c_str()))
				{
					ImGui::TextDisabled("Handle: %p", resource.Handle);
					ImGui::TextDisabled("Pointer: %p", resource.Pointer);
					ImGui::TextDisabled("Size: %d", resource.Size);
					ImGui::TextDisabled("Name: %s", resource.UnderlyingName.c_str());
					ImGui::TooltipGeneric("The real underlying name of the file.");

					if (ImGui::SmallButton("Memory Editor"))
					{
						memEditor.Open = true;
						memPtr = resource.Pointer;
						memSz = resource.Size;
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}
	void DbgTexturesTab()
	{
		if (ImGui::BeginTabItem("Textures"))
		{
			ImGui::BeginChild("##TexturesTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			std::map<std::string, Texture*>	TexRegistry = TextureService->GetRegistry();
			std::vector<QueuedTexture>		TexQueued = TextureService->GetQueuedTextures();

			float previewSize = ImGui::GetTextLineHeightWithSpacing() * 3;

			ImGui::Text("Loaded Textures:");
			for (auto& [identifier, texture] : TexRegistry)
			{
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

				ImGui::SameLine();

				ImGui::BeginChild((identifier + "##TextureDetails").c_str(), ImVec2(ImGui::GetWindowContentRegionWidth() - previewSize - 8.0f, previewSize));
				ImGui::Text("Identifier: %s", identifier.c_str());
				ImGui::TextDisabled("Dimensions: %dx%d", texture->Width, texture->Height);
				ImGui::TextDisabled("Pointer: %p", texture->Resource);
				ImGui::EndChild();
			}

			if (TexRegistry.size() == 0)
			{
				ImGui::TextDisabled("No textures loaded.");
			}
			ImGui::Separator();
			ImGui::Text("Queued Textures:");
			for (auto& qtexture : TexQueued)
			{
				if (ImGui::TreeNode(qtexture.Identifier.c_str()))
				{
					ImGui::TextDisabled("Dimensions: %dx%d", qtexture.Width, qtexture.Height);
					ImGui::TextDisabled("ReceiveCallback: %p", qtexture.Callback);
					ImGui::TextDisabled("Data: ", qtexture.Data);

					ImGui::TreePop();
				}
			}
			if (TexQueued.size() == 0)
			{
				ImGui::TextDisabled("No textures queued for loading.");
			}

			TexRegistry.clear();
			TexQueued.clear();

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}
	void DbgShortcutsTab()
	{
		if (ImGui::BeginTabItem("Shortcuts"))
		{
			{
				ImGui::BeginChild("##ShortcutsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				ImGui::Text("Shortcuts:");

				QuickAccess::Mutex.lock();
				{
					for (auto& [identifier, shortcut] : QuickAccess::Registry)
					{
						if (ImGui::TreeNode(identifier.c_str()))
						{
							ImGui::TextDisabled("Texture: %p", shortcut.TextureNormal != nullptr ? shortcut.TextureNormal->Resource : nullptr);
							ImGui::TextDisabled("Texture (Hover): %p", shortcut.TextureHover != nullptr ? shortcut.TextureHover->Resource : nullptr);
							ImGui::TextDisabled("OnClick (Keybind): %s", shortcut.Keybind.length() != 0 ? shortcut.Keybind.c_str() : "(null)");
							ImGui::TextDisabled("Tooltip: %s", shortcut.TooltipText.length() != 0 ? shortcut.TooltipText.c_str() : "(null)");
							ImGui::TextDisabled("IsHovering: %s", shortcut.IsHovering ? "true" : "false");

							ImGui::TreePop();
						}
					}
					ImGui::Separator();
					ImGui::Text("Simple shortcuts:");
					for (auto& [identifier, shortcut] : QuickAccess::RegistrySimple)
					{
						if (ImGui::TreeNode(identifier.c_str()))
						{
							ImGui::TextDisabled("Callback: %p", shortcut);

							ImGui::TreePop();
						}
					}
				}
				QuickAccess::Mutex.unlock();

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}
	void DbgLoaderTab()
	{
		if (ImGui::BeginTabItem("Loader"))
		{
			{
				ImGui::BeginChild("##LoaderTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				Loader::Mutex.lock();
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
								case EAddonState::None:							state.append("None"); break;
								case EAddonState::Loaded:						state.append("Loaded"); break;
								case EAddonState::LoadedLOCKED:					state.append("LoadedLOCKED"); break;
								case EAddonState::NotLoaded:					state.append("NotLoaded"); break;
								case EAddonState::NotLoadedDuplicate:			state.append("NotLoadedDuplicate"); break;
								case EAddonState::NotLoadedIncompatible:		state.append("NotLoadedIncompatible"); break;
								case EAddonState::NotLoadedIncompatibleAPI:		state.append("NotLoadedIncompatibleAPI"); break;
								}

								ImGui::TextDisabled(state.c_str());
								ImGui::TextDisabled("Module: %p", addon->Module);
								ImGui::TextDisabled("Module Size: %u", addon->ModuleSize);
								ImGui::TextDisabled("MD5: %s", MD5Util::ToString(addon->MD5).c_str());
								ImGui::TextDisabled("Definitions: %p", addon->Definitions);
								ImGui::Separator();
								ImGui::TextDisabled("IsFlaggedForDisable: %s", addon->IsFlaggedForDisable ? "true" : "false");
								ImGui::TextDisabled("IsPausingUpdates: %s", addon->IsPausingUpdates ? "true" : "false");
								ImGui::TextDisabled("IsFlaggedForUninstall: %s", addon->IsFlaggedForUninstall ? "true" : "false");
								ImGui::TextDisabled("IsDisabledUntilUpdate: %s", addon->IsDisabledUntilUpdate ? "true" : "false");

								if (addon->Definitions != nullptr)
								{
									if (ImGui::SmallButton("Memory Editor"))
									{
										memEditor.Open = true;
										memPtr = addon->Definitions;
										memSz = sizeof(AddonDefinition);
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
					if (ImGui::TreeNode("API Versions"))
					{
						for (auto& [version, api] : Loader::ApiDefs)
						{
							if (ImGui::TreeNode(("Version " + std::to_string(version)).c_str()))
							{
								ImGui::TextDisabled("Pointer: %p", api);
								ImGui::TextDisabled("Size: %d", Loader::GetAddonAPISize(version));

								if (ImGui::SmallButton("Memory Editor"))
								{
									memEditor.Open = true;
									memPtr = api;
									memSz = Loader::GetAddonAPISize(version);
								}

								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}
				Loader::Mutex.unlock();

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}
	void DbgAPITab()
	{

	}
	void DbgFontsTab()
	{
		if (ImGui::BeginTabItem("Fonts"))
		{
			ImGui::BeginChild("##FontsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			ImGuiIO& io = ImGui::GetIO();
			ImFontAtlas* atlas = io.Fonts;
			ImGui::PushItemWidth(120);
			for (int i = 0; i < atlas->Fonts.Size; i++)
			{
				ImFont* font = atlas->Fonts[i];
				ImGui::PushID(font);
				NodeFont(font);
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

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}

	void NodeFont(ImFont* font)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		bool font_details_opened = ImGui::TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
			font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
		ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) { io.FontDefault = font; }
		if (!font_details_opened)
			return;

		ImGui::PushFont(font);
		ImGui::Text("The quick brown fox jumps over the lazy dog");
		ImGui::PopFont();
		ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
		ImGui::SameLine(); ImGui::TooltipGeneric(
			"Note than the default embedded font is NOT meant to be scaled.\n\n"
			"Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
			"You may oversample them to get some flexibility with scaling. "
			"You can also render at multiple sizes and select which one to use at runtime.\n\n"
			"(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");
		ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
		ImGui::Text("Fallback character: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
		ImGui::Text("Ellipsis character: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
		const int surface_sqrt = (int)sqrtf((float)font->MetricsTotalSurface);
		ImGui::Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
		for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
			if (font->ConfigData)
				if (const ImFontConfig* cfg = &font->ConfigData[config_i])
					ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
						config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);
		if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
		{
			// Display all glyphs of the fonts in separate pages of 256 characters
			const ImU32 glyph_col = ImGui::GetColorU32(ImGuiCol_Text);
			for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
			{
				// Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
				// This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
				// is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
				if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
				{
					base += 4096 - 256;
					continue;
				}

				int count = 0;
				for (unsigned int n = 0; n < 256; n++)
					if (font->FindGlyphNoFallback((ImWchar)(base + n)))
						count++;
				if (count <= 0)
					continue;
				if (!ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
					continue;
				float cell_size = font->FontSize * 1;
				float cell_spacing = style.ItemSpacing.y;
				ImVec2 base_pos = ImGui::GetCursorScreenPos();
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				for (unsigned int n = 0; n < 256; n++)
				{
					// We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
					// available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
					ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
					ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
					const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
					draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
					if (glyph)
						font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
					if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
					{
						ImGui::BeginTooltip();
						ImGui::Text("Codepoint: U+%04X", base + n);
						ImGui::Separator();
						ImGui::Text("Visible: %d", glyph->Visible);
						ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
						ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
						ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
						ImGui::EndTooltip();
					}
				}
				ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
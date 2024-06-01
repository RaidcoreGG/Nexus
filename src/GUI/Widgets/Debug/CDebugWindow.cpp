#include "CDebugWindow.h"

#include "Shared.h"
#include "State.h"

#include "Events/EventHandler.h"
#include "Keybinds/KeybindHandler.h"
#include "DataLink/DataLink.h"
#include "Textures/TextureLoader.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"
#include "Loader/Loader.h"

#include "imgui.h"
#include "imgui_extensions.h"
#include "imgui_memory_editor.h"

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
			{
				ImGui::BeginChild("##EventsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				Events::Mutex.lock();
				{
					for (auto& [identifier, ev] : Events::Registry)
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
				}
				Events::Mutex.unlock();

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}
	void DbgKeybindsTab()
	{
		if (ImGui::BeginTabItem("Keybinds"))
		{
			{
				ImGui::BeginChild("##KeybindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				Keybinds::Mutex.lock();
				{
					for (auto& [identifier, keybind] : Keybinds::Registry)
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
				}
				Keybinds::Mutex.unlock();

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}
	void DbgDataLinkTab()
	{
		if (ImGui::BeginTabItem("DataLink"))
		{
			{
				ImGui::BeginChild("##DataLinkTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				DataLink::Mutex.lock();
				{
					for (auto& [identifier, resource] : DataLink::Registry)
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
				}
				DataLink::Mutex.unlock();

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}
	void DbgTexturesTab()
	{
		if (ImGui::BeginTabItem("Textures"))
		{
			{
				ImGui::BeginChild("##TexturesTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				TextureLoader::Mutex.lock();
				{
					ImGui::Text("Loaded Textures:");
					for (auto& [identifier, texture] : TextureLoader::Registry)
					{
						if (ImGui::TreeNode(identifier.c_str()))
						{
							ImGui::TextDisabled("Dimensions: %dx%d", texture->Width, texture->Height);
							ImGui::TextDisabled("Pointer: %p", texture->Resource);

							ImGui::TreePop();
						}
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
					}
					if (TextureLoader::Registry.size() == 0)
					{
						ImGui::TextDisabled("No textures loaded.");
					}
					ImGui::Separator();
					ImGui::Text("Queued Textures:");
					for (auto& qtexture : TextureLoader::QueuedTextures)
					{
						if (ImGui::TreeNode(qtexture.Identifier.c_str()))
						{
							ImGui::TextDisabled("Dimensions: %dx%d", qtexture.Width, qtexture.Height);
							ImGui::TextDisabled("ReceiveCallback: %p", qtexture.Callback);
							ImGui::TextDisabled("Data: ", qtexture.Data);

							ImGui::TreePop();
						}
					}
					if (TextureLoader::QueuedTextures.size() == 0)
					{
						ImGui::TextDisabled("No textures queued for loading.");
					}
				}
				TextureLoader::Mutex.unlock();

				ImGui::EndChild();
			}

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
}
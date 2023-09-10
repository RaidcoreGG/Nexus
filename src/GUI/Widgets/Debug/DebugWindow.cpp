#include "DebugWindow.h"

namespace GUI
{
	static ImGui::MemoryEditor memEditor;
	void* memPtr = nullptr;
	size_t memSz = 0;

	DebugWindow::DebugWindow()
	{
		MumbleWindow = new MumbleOverlay();
	}

	void DebugWindow::Render()
	{
		MumbleWindow->Render();

		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin("Debug", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
			ImGui::Checkbox("Show Mumble overlay", &MumbleWindow->Visible);
			ImGui::PopStyleVar();

			if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Events"))
				{
					{
						ImGui::BeginChild("##EventsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						Events::Mutex.lock();
						{
							for (auto& [identifier, subscribers] : Events::Registry)
							{
								if (ImGui::TreeNode(identifier.c_str()))
								{
									if (subscribers.size() == 0)
									{
										ImGui::TextDisabled("This event has no subscribers.");
									}
									else
									{
										ImGui::TextDisabled("Subscribers:");
										for (EVENT_CONSUME callback : subscribers)
										{
											ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabled("%p", callback);
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
									ImGui::TextDisabled("Texture: %p", shortcut.TextureNormal->Resource);
									ImGui::TextDisabled("Texture (Hover): %p", shortcut.TextureHover->Resource);
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
				if (ImGui::BeginTabItem("Loader"))
				{
					{
						ImGui::BeginChild("##LoaderTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						Loader::Mutex.lock();
						{
							if (ImGui::TreeNode("Loaded"))
							{
								for (const auto& [path, addon] : Loader::AddonDefs)
								{
									ImGui::TextDisabled("%s", path.string().c_str());
								}
								ImGui::TreePop();
							}
							if (ImGui::TreeNode("Blacklisted"))
							{
								for (auto& path : Loader::Blacklist)
								{
									ImGui::TextDisabled("%s", path.string().c_str());
								}
								ImGui::TreePop();
							}
							ImGui::TooltipGeneric("These files will not be checked during the next wave of addon loading.");
						}
						Loader::Mutex.unlock();

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
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

		memEditor.DrawWindow("Memory Editor", memPtr, memSz);
	}
}
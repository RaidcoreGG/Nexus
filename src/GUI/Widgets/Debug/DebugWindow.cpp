#include "DebugWindow.h"

#include "../../../Shared.h"
#include "../../../State.h"

#include "../../../Events/EventHandler.h"
#include "../../../Keybinds/KeybindHandler.h"
#include "../../../DataLink/DataLink.h"
#include "../../../Textures/TextureLoader.h"

namespace GUI
{
	void DebugWindow::Render()
	{
        if (!Visible) { return; }

        ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
        if (ImGui::Begin("Debug", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Events"))
                {
                    {
                        ImGui::BeginChild("##EventsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

                        Events::Mutex.lock();
                        for (auto& [identifier, subscribers] : Events::Registry)
                        {
                            if (ImGui::TreeNode(identifier.c_str()))
                            {
                                if (subscribers.size() == 0)
                                {
                                    ImGui::TextDisabled("No addon is subscribed to this event.");
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
                        for (auto& [identifier, handler] : Keybinds::HandlerRegistry)
                        {
                            ImGui::Text(identifier.c_str()); ImGui::SameLine(); ImGui::TextDisabled("%p", handler);
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
                        for (auto& [identifier, resource] : DataLink::Registry)
                        {
                            if (ImGui::TreeNode(identifier.c_str()))
                            {
                                ImGui::TextDisabled("Handle: %p", resource.Handle);
                                ImGui::TextDisabled("Pointer: %p", resource.Pointer);
                                ImGui::TextDisabled("Size: %d", resource.Size);

                                ImGui::TreePop();
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
                        ImGui::Text("Loaded Textures:");
                        for (auto& [identifier, texture] : TextureLoader::Registry)
                        {
                            if (ImGui::TreeNode(identifier.c_str()))
                            {
                                ImGui::TextDisabled("Dimensions: %dx%d", texture.Width, texture.Height);
                                ImGui::TextDisabled("Pointer: %p", texture.Resource);

                                ImGui::TreePop();
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
                        TextureLoader::Mutex.unlock();

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
	}

	void DebugWindow::MenuOption(int aCategory)
	{
		if (aCategory == 1)
		{
			ImGui::ToggleButton("Debug Info", &Visible, ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
		}
	}
}
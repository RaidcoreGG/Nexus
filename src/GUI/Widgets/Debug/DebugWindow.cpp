#include "DebugWindow.h"

#include "../../../Shared.h"
#include "../../../State.h"

#include "../../../Events/EventHandler.h"
#include "../../../DataLink/DataLink.h"

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

                        EventHandler::EventRegistryMutex.lock();
                        for (auto& [identifier, subscribers] : EventHandler::EventRegistry)
                        {
                            if (ImGui::TreeNode(identifier.c_str()))
                            {
                                for (EVENT_CONSUME callback : subscribers)
                                {
                                    ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabled("%p", callback);
                                }
                                ImGui::TreePop();
                            }
                        }
                        EventHandler::EventRegistryMutex.unlock();

                        ImGui::EndChild();
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("DataLink"))
                {
                    {
                        ImGui::BeginChild("##DataLinkTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

                        DataLink::DataLinkMutex.lock();
                        for (auto& [identifier, resource] : DataLink::DataLinkRegistry)
                        {
                            ImGui::Text(identifier.c_str()); ImGui::SameLine(); ImGui::TextDisabled("%p", resource.Pointer);
                        }
                        DataLink::DataLinkMutex.unlock();

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
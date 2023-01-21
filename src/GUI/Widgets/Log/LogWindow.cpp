#include "LogWindow.h"

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../Logging/LogHandler.h"

namespace GUI
{
    bool SelectedOnly = false;

    void LogWindow::Render()
    {
        if (!Visible) { return; }

        ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
        if (ImGui::Begin("Log", &Visible, ImGuiWindowFlags_AlwaysAutoResize))
        {

            const char* items[] = { "Critical", "Warning", "Info", "Debug", "Trace", "All" };
            static int selectedLevel = (int)(ELogLevel::ALL)-1;
            ImGui::Text("Filter: "); ImGui::SameLine();
            ImGui::Combo("", &selectedLevel, items, IM_ARRAYSIZE(items));

            ELogLevel filterLevel = (ELogLevel)(selectedLevel + 1);

            ImGui::SameLine();
            ImGui::Checkbox("Selected level only", &SelectedOnly);

            ImGui::Separator();

            {
                ImGui::BeginChild("logmessages", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), ImGuiWindowFlags_AlwaysVerticalScrollbar);
                
                MessageMutex.lock();
                /* Show last 200 log messages */
                size_t start = 0;
                if (Logger->LogEntries.size() > 200) { start = Logger->LogEntries.size() - 200; }

                for (size_t i = start; i < Logger->LogEntries.size(); i++)
                {
                    LogEntry entry = Logger->LogEntries[i];

                    if ((filterLevel == ELogLevel::ALL) ||
                        (SelectedOnly && entry.LogLevel == filterLevel) ||
                        (!SelectedOnly && entry.LogLevel <= filterLevel))
                    {
                        const char* level;
                        switch (entry.LogLevel)
                        {
                        case ELogLevel::CRITICAL:   level = "[CRITICAL]";   ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));     break;
                        case ELogLevel::WARNING:    level = "[WARNING]";    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));   break;
                        case ELogLevel::INFO:       level = "[INFO]";       ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));     break;
                        case ELogLevel::DEBUG:      level = "[DEBUG]";      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 148, 255, 255));   break;

                        default:                    level = "[TRACE]";      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255)); break;
                        }

                        float off1 = ImGui::CalcTextSize("XXXXXXXXX").x;
                        float off2 = ImGui::CalcTextSize("XXXXXXXXXX").x;

                        /* time */
                        ImGui::TextW(entry.TimestampString(false).c_str()); ImGui::SameLine(off1);

                        /* level */
                        ImGui::Text(level); ImGui::SameLine(off1 + off2);

                        /* message */
                        ImGui::TextWrappedW(entry.Message.c_str());

                        ImGui::PopStyleColor();
                    }
                }
                MessageMutex.unlock();

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }

                ImGui::EndChild();
            }
            
        }
        ImGui::End();
    }

    void LogWindow::MenuOption(int aCategory)
    {
        if (aCategory == 1)
        {
            ImGui::ToggleButton("Log", &Visible, ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
        }
    }

    void LogWindow::LogMessage(LogEntry aLogEntry) {}
}
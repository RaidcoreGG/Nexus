#include "LogWindow.h"

namespace GUI
{
	float lwWidth = 38.0f;
	float lwHeight = 24.0f;

	bool SelectedOnly = false;
	std::string ChannelFilter;

	float off1 = 0.0f;
	float off2 = 0.0f;
	const char* filterLevels[] = { "Critical", "Warning", "Info", "Debug", "Trace", "All" };
	size_t amtShown = 0;

	LogWindow::LogWindow(std::string aName, ELogLevel aLogLevel)
	{
		Name = aName;
		LogLevel = aLogLevel;
	}

	void LogWindow::Render()
	{
		if (!Visible) { return; }

		if (!(off1 && off2))
		{
			off1 = ImGui::CalcTextSize("XXXXXXXXX").x;
			off2 = ImGui::CalcTextSize("XXXXXXXXXXX").x;
		}

		ImGui::SetNextWindowSize(ImVec2(lwWidth * ImGui::GetFontSize(), lwHeight * ImGui::GetFontSize()));
		if (ImGui::Begin(Name.c_str(), &Visible, WindowFlags_Default))
		{
			static int selectedLevel = (int)(ELogLevel::ALL)-1;
			ImGui::Text("Filter: "); ImGui::SameLine();
			ImGui::Combo("##filterLogLevel", &selectedLevel, filterLevels, IM_ARRAYSIZE(filterLevels));

			ELogLevel filterLevel = (ELogLevel)(selectedLevel + 1);

			ImGui::SameLine();
			ImGui::Checkbox("Selected level only", &SelectedOnly);

			float windowWidthQuarter = ImGui::GetWindowContentRegionWidth() / 4.0f;

			ImGui::Separator();
			{
				ImGui::BeginChild("logchannels", ImVec2(windowWidthQuarter, 0.0f));

				ImGui::Text("Channel:");

				LogHandler::Mutex.lock();
				{
					for (std::string ch : Channels)
					{
						float opacity = 0.8f;
						if (ch == ChannelFilter)
						{
							opacity = 1.0f;
						}
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
						if (ImGui::Button(ch.c_str(), ImVec2(windowWidthQuarter, 0.0f)))
						{
							/* if the selected channel is already the set filter, reset filter */
							if (ChannelFilter == ch)
							{
								ChannelFilter = "";
							}
							else
							{
								ChannelFilter = ch.c_str();
							}
						}
						ImGui::PopStyleVar();
					}
				}
				LogHandler::Mutex.unlock();

				ImGui::TextDisabled("Showing %d out of %d", amtShown, LogEntries.size() > 400 ? 400 : LogEntries.size());

				ImGui::EndChild();
			}

			ImGui::SameLine();

			amtShown = 0;

			{
				ImGui::BeginChild("logmessages", ImVec2(windowWidthQuarter * 3 - 1, 0.0f));
				
				LogHandler::Mutex.lock();
				{
					/* Show last 400 log messages */
					size_t start = 0;
					if (LogEntries.size() > 400) { start = LogEntries.size() - 400; }

					for (size_t i = start; i < LogEntries.size(); i++)
					{
						LogEntry entry = LogEntries[i];

						if (((filterLevel == ELogLevel::ALL) ||
							(SelectedOnly && entry.LogLevel == filterLevel) ||
							(!SelectedOnly && entry.LogLevel <= filterLevel)) &&
							((ChannelFilter == "") || ChannelFilter == entry.Channel))
						{
							amtShown++;

							const char* level;
							switch (entry.LogLevel)
							{
								case ELogLevel::CRITICAL:   level = "[CRITICAL]";   ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));     break;
								case ELogLevel::WARNING:    level = "[WARNING]";    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));   break;
								case ELogLevel::INFO:       level = "[INFO]";       ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));     break;
								case ELogLevel::DEBUG:      level = "[DEBUG]";      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 148, 255, 255));   break;

								default:                    level = "[TRACE]";      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255)); break;
							}

							/* time */
							ImGui::Text(entry.TimestampString(false).c_str()); ImGui::SameLine(off1);

							/* level */
							ImGui::Text(level); ImGui::SameLine(off1 + off2);

							/* message */
							ImGui::TextWrapped(entry.Message.c_str());

							ImGui::PopStyleColor();
						}
					}
				}
				LogHandler::Mutex.unlock();

				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				{
					ImGui::SetScrollHereY(1.0f);
				}

				ImGui::EndChild();
			}
		}
		ImGui::End();
	}

	void LogWindow::LogMessage(LogEntry aLogEntry) {}
}
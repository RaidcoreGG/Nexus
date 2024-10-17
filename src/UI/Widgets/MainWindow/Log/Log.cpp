///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Log.cpp
/// Description  :  Contains the content of the log window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Log.h"

#include <regex>

#include "imgui_extensions.h"

#include "resource.h"

CLogWindow::CLogWindow()
{
	this->Name           = "Log";
	this->DisplayName    = "((000006))";
	this->IconIdentifier = "ICON_LOG";
	this->IconID         = RES_ICON_LOG;
	this->LogLevel       = ELogLevel::ALL;
}

void CLogWindow::RenderContent()
{
	static int selectedLevel = (int)(ELogLevel::ALL)-1;
	static const char* filterLevels[] = { "Critical", "Warning", "Info", "Debug", "Trace", "All" };
	static bool selectedOnly = false;
	static std::string channelFilter;
	static size_t amtShown = 0;

	float off1 = ImGui::CalcTextSize("XX:XX:XX.XXX  ").x;
	float off2 = ImGui::CalcTextSize("XXXXXXXXXXX").x;

	ImGui::Text("Filter: "); ImGui::SameLine();
	ImGui::Combo("##Filter_LogLevel", &selectedLevel, filterLevels, IM_ARRAYSIZE(filterLevels));

	ELogLevel filterLevel = (ELogLevel)(selectedLevel + 1);

	ImGui::SameLine();
	ImGui::Checkbox("Selected level only", &selectedOnly);

	float windowWidthQuarter = ImGui::GetWindowContentRegionWidth() / 4.0f;

	ImGui::Separator();
	{
		ImGui::BeginChild("##LogChannels", ImVec2(windowWidthQuarter, 0.0f));

		ImGui::Text("Channel:");

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (std::string ch : Channels)
			{
				float opacity = 0.8f;
				if (ch == channelFilter)
				{
					opacity = 1.0f;
				}
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
				if (ImGui::Button(ch.c_str(), ImVec2(windowWidthQuarter, 0.0f)))
				{
					/* if the selected channel is already the set filter, reset filter */
					if (channelFilter == ch)
					{
						channelFilter = "";
					}
					else
					{
						channelFilter = ch.c_str();
					}
				}
				ImGui::PopStyleVar();
			}
		}
		ImGui::TextDisabled("Showing %d out of %d", amtShown, LogEntries.size());

		ImGui::EndChild();
	}

	ImGui::SameLine();

	amtShown = 0;

	{
		ImGui::BeginChild("logmessages", ImVec2(windowWidthQuarter * 3 - 1, 0.0f));

		ImGuiStyle& style = ImGui::GetStyle();
		float wrapWidth = ImGui::GetContentRegionAvailWidth() - off1 - off2 - style.ScrollbarSize;
		float maxHeight = ImGui::GetWindowHeight();

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			/* Show last 400 log messages matching filter */
			
			std::vector<DisplayLogEntry*> displayedEntries;
			for (size_t i = 0; i < LogEntries.size(); i++)
			{
				DisplayLogEntry* msg = LogEntries[i];

				if (((filterLevel == ELogLevel::ALL) ||
					(selectedOnly && msg->Entry->LogLevel == filterLevel) ||
					(!selectedOnly && msg->Entry->LogLevel <= filterLevel)) &&
					((channelFilter == "") || channelFilter == msg->Entry->Channel))
				{
					displayedEntries.push_back(msg);
				}
			}

			size_t start = 0;
			if (displayedEntries.size() > 400) { start = displayedEntries.size() - 400; }

			if (ImGui::BeginTable("##LogMessages", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit))
			{
				for (size_t i = start; i < displayedEntries.size(); i++)
				{
					DisplayLogEntry* msg = displayedEntries[i];

					amtShown++;

					const char* level;
					ImColor levelColor;
					switch (msg->Entry->LogLevel)
					{
						case ELogLevel::CRITICAL:   level = "[CRITICAL]";   levelColor = IM_COL32(255, 0, 0, 255);     break;
						case ELogLevel::WARNING:    level = "[WARNING]";    levelColor = IM_COL32(255, 255, 0, 255);   break;
						case ELogLevel::INFO:       level = "[INFO]";       levelColor = IM_COL32(0, 255, 0, 255);     break;
						case ELogLevel::DEBUG:      level = "[DEBUG]";      levelColor = IM_COL32(0, 148, 255, 255);   break;

						default:                    level = "[TRACE]";      levelColor = IM_COL32(220, 220, 220, 255); break;
					}

					ImVec2 rowPos = ImGui::GetCursorPos();

					ImGui::TableNextRow();

					/* time */
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(levelColor, msg->Entry->TimestampString(false, true).c_str());

					/* level */
					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(levelColor, level);

					float msgHeight = ImGui::CalcTextSize(msg->Entry->Message.c_str(), (const char*)0, false, wrapWidth).y;

					ImGui::TableSetColumnIndex(2);

					/*  above visible space                        || under visible space */
					if (rowPos.y < ImGui::GetScrollY() - msgHeight || rowPos.y > ImGui::GetScrollY() + maxHeight)
					{
						ImGui::Dummy(ImVec2(wrapWidth, msgHeight));
						continue;
					}

					/* message */
					float lineHeight = ImGui::GetTextLineHeight();
					ImVec2 posInitial = ImGui::GetCursorPos();
					ImVec2 pos = posInitial;
					
					int colStack = 0;

					for (MessagePart& msgPart : msg->Parts)
					{
						switch (msgPart.Type)
						{
							case EMessagePartType::Text:
							{
								float currWidth = ImGui::CalcTextSize(msgPart.Text.c_str()).x;

								if (pos.x - posInitial.x + currWidth > wrapWidth)
								{
									pos.x = posInitial.x;
									pos.y += lineHeight;
								}

								ImGui::SetCursorPos(pos);
								ImGui::TextUnformatted(msgPart.Text.c_str());
								pos.x += currWidth;
								break;
							}
							case EMessagePartType::ColorPush:
							{
								ImGui::PushStyleColor(ImGuiCol_Text, msgPart.Color);
								colStack++;
								break;
							}
							case EMessagePartType::ColorPop:
							{
								if (colStack > 0)
								{
									ImGui::PopStyleColor();
									colStack--;
								}
								break;
							}
							case EMessagePartType::LineBreak:
							{
								pos.x = posInitial.x;
								pos.y += lineHeight;
								break;
							}
						}
					}

					// cleanup
					while (colStack > 0)
					{
						ImGui::PopStyleColor();
						colStack--;
					}
				}

				ImGui::EndTable();
			}
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
	}
}

void CLogWindow::LogMessage(LogEntry* aLogEntry)
{
	const std::lock_guard<std::mutex> lock(Mutex);

	if (std::find(this->Channels.begin(), this->Channels.end(), aLogEntry->Channel) == this->Channels.end())
	{
		this->Channels.push_back(aLogEntry->Channel);
	}

	DisplayLogEntry* displayMsg = nullptr;

	if (aLogEntry->RepeatCount > 1)
	{
		displayMsg = this->LogEntries[this->LogEntries.size() - 1];
		displayMsg->Entry->RepeatCount = aLogEntry->RepeatCount;
		displayMsg->Parts.clear();
	}
	else
	{
		displayMsg = new DisplayLogEntry();
		displayMsg->Entry = aLogEntry;
	}

	if (displayMsg->Entry->RepeatCount > 1)
	{
		MessagePart msgPart{};
		msgPart.Type = EMessagePartType::Text;
		msgPart.Text = "(" + std::to_string(displayMsg->Entry->RepeatCount) + ")";
		displayMsg->Parts.push_back(msgPart);
		MessagePart msgSpace{};
		msgSpace.Type = EMessagePartType::Text;
		msgSpace.Text = " ";
		displayMsg->Parts.push_back(msgSpace);
	}

	size_t idxLastWordStart = 0;
	for (size_t i = 0; i < aLogEntry->Message.size(); i++)
	{
		size_t remainingLength = aLogEntry->Message.size() - i - 1; // helper to quickly check against possibility of a tag existing

		if (aLogEntry->Message[i] == ' ')
		{
			MessagePart msgPart{};
			msgPart.Type = EMessagePartType::Text;
			msgPart.Text = aLogEntry->Message.substr(idxLastWordStart, i - idxLastWordStart);
			displayMsg->Parts.push_back(msgPart);

			MessagePart msgSpace{};
			msgSpace.Type = EMessagePartType::Text;
			msgSpace.Text = " ";
			displayMsg->Parts.push_back(msgSpace);

			idxLastWordStart = i + 1;
		}
		else if (aLogEntry->Message[i] == '\n')
		{
			MessagePart msgPartPre{};
			msgPartPre.Type = EMessagePartType::Text;
			msgPartPre.Text = aLogEntry->Message.substr(idxLastWordStart, i - idxLastWordStart);
			displayMsg->Parts.push_back(msgPartPre);

			MessagePart msgPart{};
			msgPart.Type = EMessagePartType::LineBreak;
			displayMsg->Parts.push_back(msgPart);

			idxLastWordStart = i + 1;
		}
		// magic number 10 because a string like "<c=#123456>" is 11 chars long
		else if (remainingLength >= 10 &&
				aLogEntry->Message[i] == '<' &&
				aLogEntry->Message[i + 1] == 'c' &&
				aLogEntry->Message[i + 2] == '=' &&
				aLogEntry->Message[i + 3] == '#' &&
				aLogEntry->Message[i + 10] == '>')
		{
			std::string hexCol = aLogEntry->Message.substr(i + 4, 6);

			if (std::regex_match(hexCol, std::regex("[0-9a-fA-F]{6}")))
			{
				ImVec4 col = ImGui::HEXtoIV4(hexCol.c_str());

				MessagePart msgPartPre{};
				msgPartPre.Type = EMessagePartType::Text;
				msgPartPre.Text = aLogEntry->Message.substr(idxLastWordStart, i - idxLastWordStart);
				displayMsg->Parts.push_back(msgPartPre);

				MessagePart msgPart{};
				msgPart.Type = EMessagePartType::ColorPush;
				msgPart.Color = col;
				displayMsg->Parts.push_back(msgPart);

				i += 10; // 10, because 1 will be auto-incremented -> 11 skipped
				idxLastWordStart = i + 1;
			}
		}
		// magic number 3 because a string like "</c>" is 4 chars long
		else if (remainingLength >= 3 &&
				 aLogEntry->Message[i] == '<' &&
				 aLogEntry->Message[i + 1] == '/' &&
				 aLogEntry->Message[i + 2] == 'c' &&
				 aLogEntry->Message[i + 3] == '>')
		{
			MessagePart msgPartPre{};
			msgPartPre.Type = EMessagePartType::Text;
			msgPartPre.Text = aLogEntry->Message.substr(idxLastWordStart, i - idxLastWordStart);
			displayMsg->Parts.push_back(msgPartPre);

			MessagePart msgPart{};
			msgPart.Type = EMessagePartType::ColorPop;
			displayMsg->Parts.push_back(msgPart);

			i += 3; // 3, because 1 will be auto-incremented -> 4 skipped
			idxLastWordStart = i + 1;
		}
		else if (i == aLogEntry->Message.size() - 1)
		{
			MessagePart msgPart{};
			msgPart.Type = EMessagePartType::Text;
			msgPart.Text = aLogEntry->Message.substr(idxLastWordStart, i - idxLastWordStart + 1); // +1 because this char is included
			displayMsg->Parts.push_back(msgPart);
		}
	}

	if (aLogEntry->RepeatCount == 1)
	{
		this->LogEntries.push_back(displayMsg);
	}
}

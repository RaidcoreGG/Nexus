#include "CLogWindow.h"

#include <regex>

#include "Shared.h"
#include "Index.h"
#include "State.h"

#include "Services/Logging/LogHandler.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

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

	CLogWindow::CLogWindow(std::string aName, ELogLevel aLogLevel)
	{
		Name = aName;
		LogLevel = aLogLevel;
	}

	void CLogWindow::Render()
	{
		if (!Visible) { return; }

		if (!(off1 && off2))
		{
			off1 = ImGui::CalcTextSize("XX:XX:XX.XXX  ").x;
			off2 = ImGui::CalcTextSize("XXXXXXXXXXX").x;
		}

		ImGui::SetNextWindowSize(ImVec2(lwWidth * ImGui::GetFontSize(), lwHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
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

				const std::lock_guard<std::mutex> lock(Mutex);
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
				ImGui::TextDisabled("Showing %d out of %d", amtShown, LogEntries.size() > 400 ? 400 : LogEntries.size());

				ImGui::EndChild();
			}

			ImGui::SameLine();

			amtShown = 0;

			{
				ImGui::BeginChild("logmessages", ImVec2(windowWidthQuarter * 3 - 1, 0.0f));
				
				ImGuiStyle& style = ImGui::GetStyle();
				float wrapWidth = ImGui::GetContentRegionAvailWidth() - off1 - off2 - style.ScrollbarSize;

				const std::lock_guard<std::mutex> lock(Mutex);
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
							ImColor levelColor;
							switch (entry.LogLevel)
							{
								case ELogLevel::CRITICAL:   level = "[CRITICAL]";   levelColor = IM_COL32(255, 0, 0, 255);     break;
								case ELogLevel::WARNING:    level = "[WARNING]";    levelColor = IM_COL32(255, 255, 0, 255);   break;
								case ELogLevel::INFO:       level = "[INFO]";       levelColor = IM_COL32(0, 255, 0, 255);     break;
								case ELogLevel::DEBUG:      level = "[DEBUG]";      levelColor = IM_COL32(0, 148, 255, 255);   break;

								default:                    level = "[TRACE]";      levelColor = IM_COL32(220, 220, 220, 255); break;
							}

							/* time */
							ImGui::TextColored(levelColor, entry.TimestampString(false, true).c_str()); ImGui::SameLine(off1);

							/* level */
							ImGui::TextColored(levelColor, level); ImGui::SameLine(off1 + off2);

							float msgHeight = ImGui::CalcTextSize(entry.Message.c_str(), (const char*)0, false, wrapWidth).y;

							/* message divider */
							ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, (ImVec4)levelColor);
							if (ImGui::BeginTable(("##" + entry.Message).c_str(), 1, ImGuiTableFlags_Borders, ImVec2(1.0f, msgHeight)))
							{
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("");
								ImGui::EndTable();
							}
							ImGui::SameLine();
							ImGui::PopStyleColor();

							enum class EMessagePartType {
								NONE,
								Text,
								ColorPush,
								ColorPop,
								LineBreak
							};

							struct MessagePart
							{
								EMessagePartType Type;
								std::string Text;
								ImVec4 Color;
							};

							/* message */
							float lineHeight = ImGui::GetTextLineHeight();
							ImVec2 posInitial = ImGui::GetCursorPos();
							ImVec2 pos = posInitial;
							std::vector<MessagePart> msgParts; // = String::Split(entry.Message, " ", true);

							size_t idxLastWordStart = 0;
							for (size_t i = 0; i < entry.Message.size(); i++)
							{
								size_t remainingLength = entry.Message.size() - i - 1; // helper to quickly check against possibility of a tag existing

								if (entry.Message[i] == ' ')
								{
									MessagePart msgPart{};
									msgPart.Type = EMessagePartType::Text;
									msgPart.Text = entry.Message.substr(idxLastWordStart, i - idxLastWordStart);
									msgParts.push_back(msgPart);

									MessagePart msgSpace{};
									msgSpace.Type = EMessagePartType::Text;
									msgSpace.Text = " ";
									msgParts.push_back(msgSpace);

									idxLastWordStart = i + 1;
								}
								else if (entry.Message[i] == '\n')
								{
									MessagePart msgPartPre{};
									msgPartPre.Type = EMessagePartType::Text;
									msgPartPre.Text = entry.Message.substr(idxLastWordStart, i - idxLastWordStart);
									msgParts.push_back(msgPartPre);

									MessagePart msgPart{};
									msgPart.Type = EMessagePartType::LineBreak;
									msgParts.push_back(msgPart);

									idxLastWordStart = i + 1;
								}
								// magic number 10 because a string like "<c=#123456>" is 11 chars long
								else if (remainingLength >= 10 &&
									entry.Message[i] == '<' &&
									entry.Message[i + 1] == 'c' &&
									entry.Message[i + 2] == '=' &&
									entry.Message[i + 3] == '#' &&
									entry.Message[i + 10] == '>')
								{
									std::string hexCol = entry.Message.substr(i + 4, 6);

									if (std::regex_match(hexCol, std::regex("[0-9a-fA-F]{6}")))
									{
										ImVec4 col = ImGui::HEXtoIV4(hexCol.c_str());

										MessagePart msgPartPre{};
										msgPartPre.Type = EMessagePartType::Text;
										msgPartPre.Text = entry.Message.substr(idxLastWordStart, i - idxLastWordStart);
										msgParts.push_back(msgPartPre);

										MessagePart msgPart{};
										msgPart.Type = EMessagePartType::ColorPush;
										msgPart.Color = col;
										msgParts.push_back(msgPart);

										i += 10; // 10, because 1 will be auto-incremented -> 11 skipped
										idxLastWordStart = i + 1;
									}
								}
								// magic number 3 because a string like "</c>" is 4 chars long
								else if (remainingLength >= 3 &&
									entry.Message[i] == '<' &&
									entry.Message[i + 1] == '/' &&
									entry.Message[i + 2] == 'c' &&
									entry.Message[i + 3] == '>')
								{
									MessagePart msgPartPre{};
									msgPartPre.Type = EMessagePartType::Text;
									msgPartPre.Text = entry.Message.substr(idxLastWordStart, i - idxLastWordStart);
									msgParts.push_back(msgPartPre);

									MessagePart msgPart{};
									msgPart.Type = EMessagePartType::ColorPop;
									msgParts.push_back(msgPart);

									i += 3; // 3, because 1 will be auto-incremented -> 4 skipped
									idxLastWordStart = i + 1;
								}
								else if (i == entry.Message.size() - 1)
								{
									MessagePart msgPart{};
									msgPart.Type = EMessagePartType::Text;
									msgPart.Text = entry.Message.substr(idxLastWordStart, i - idxLastWordStart + 1); // +1 because this char is included
									msgParts.push_back(msgPart);
								}
							}

							int colStack = 0;
							float currWidth = 0.0f; // thanks to C2360 this has to be up here, despite being a local

							for (MessagePart msgPart : msgParts)
							{
								switch (msgPart.Type)
								{
								case EMessagePartType::Text:
									currWidth = ImGui::CalcTextSize(msgPart.Text.c_str()).x;

									if (pos.x - posInitial.x + currWidth > wrapWidth)
									{
										pos.x = posInitial.x;
										pos.y += lineHeight;
									}

									ImGui::SetCursorPos(pos);
									ImGui::TextUnformatted(msgPart.Text.c_str());
									pos.x += currWidth;
									break;
								case EMessagePartType::ColorPush:
									ImGui::PushStyleColor(ImGuiCol_Text, msgPart.Color);
									colStack++;
									break;
								case EMessagePartType::ColorPop:
									if (colStack > 0)
									{
										ImGui::PopStyleColor();
										colStack--;
									}
									break;
								case EMessagePartType::LineBreak:
									pos.x = posInitial.x;
									pos.y += lineHeight;
									break;
								}
							}

							// cleanup
							while (colStack > 0)
							{
								ImGui::PopStyleColor();
								colStack--;
							}

							ImGui::Separator();
						}
					}
				}

				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				{
					ImGui::SetScrollHereY(1.0f);
				}

				ImGui::EndChild();
			}
		}
		ImGui::End();
	}

	void CLogWindow::LogMessage(LogEntry aLogEntry)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		if (std::find(this->Channels.begin(), this->Channels.end(), aLogEntry.Channel) == this->Channels.end())
		{
			this->Channels.push_back(aLogEntry.Channel);
		}

		this->LogEntries.push_back(aLogEntry);
	}
}

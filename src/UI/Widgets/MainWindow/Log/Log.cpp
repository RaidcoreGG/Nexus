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

#include "Context.h"
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
	if (this->IsInvalid)
	{
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CEscapeClosing* escclose = uictx->GetEscapeClosingService();

		escclose->Deregister(this->GetVisibleStatePtr());
		escclose->Register(this->GetNameID().c_str(), this->GetVisibleStatePtr());

		this->IsInvalid = false;
	}

	float off1 = ImGui::CalcTextSize("XX:XX:XX.XXX  ").x;
	float off2 = ImGui::CalcTextSize("XXXXXXXXXXX").x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Shown messages");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(ImGui::CalcTextSize("######").x);
	if (ImGui::InputScalar("##Filter_MaxMessages", ImGuiDataType_S32, &this->MaxShownCount))
	{
		this->MaxShownCount = abs(this->MaxShownCount);
		if (this->MaxShownCount > 99999)
		{
			this->MaxShownCount = 99999;
		}
	}
	ImGui::TooltipGeneric("Set to 0 to show all messages.");

	ImGui::AlignTextToFramePadding();
	ImGui::SameLine();
	ImGui::Text("Log Level");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::CalcTextSize("##########").x);
	static std::string levelPreview = "All";
	if (ImGui::BeginCombo("##Filter_LogLevel", levelPreview.c_str()))
	{
		if (ImGui::Selectable("Critical"))
		{
			levelPreview = "Critical";
			this->FilterLevel = ELogLevel::CRITICAL;
		}

		if (ImGui::Selectable("Warning"))
		{
			levelPreview = "Warning";
			this->FilterLevel = ELogLevel::WARNING;
		}

		if (ImGui::Selectable("Info"))
		{
			levelPreview = "Info";
			this->FilterLevel = ELogLevel::INFO;
		}

		if (ImGui::Selectable("Debug"))
		{
			levelPreview = "Debug";
			this->FilterLevel = ELogLevel::DEBUG;
		}

		if (ImGui::Selectable("Trace"))
		{
			levelPreview = "Trace";
			this->FilterLevel = ELogLevel::TRACE;
		}

		if (ImGui::Selectable("All"))
		{
			levelPreview = "All";
			this->FilterLevel = ELogLevel::ALL;
		}

		ImGui::EndCombo();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Selected level only", &this->SelectedLevelOnly);
	ImGui::TooltipGeneric("If selected only messages with the specific log level are shown.\nIf not selected, all messages with a log level higher or equal to the filter are shown.");

	ImGuiStyle& style = ImGui::GetStyle();

	float widthChannels = ImGui::GetWindowContentRegionWidth() * .25f;
	float widthMessages = ImGui::GetWindowContentRegionWidth() * .75f - style.ItemSpacing.x;

	std::vector<std::string> activeChannels;

	static int amtShown = 0;

	ImGui::Separator();
	{
		ImGui::BeginChild("Channels", ImVec2(widthChannels, 0.0f), false, ImGuiWindowFlags_NoBackground);

		ImGui::Text("Channels");

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			for (LogChannel& ch : this->Channels)
			{
				float opacity = 0.8f;
				if (ch.IsSelected)
				{
					opacity = 1.0f;
					activeChannels.push_back(ch.Name);
				}
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
				if (ImGui::Button(ch.Name.c_str(), ImVec2(widthChannels, 0.0f)))
				{
					ch.IsSelected = !ch.IsSelected;
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
		ImGui::BeginChild("Messages", ImVec2(widthMessages, 0.0f), false, ImGuiWindowFlags_NoBackground);

		float wrapWidth = ImGui::GetContentRegionAvailWidth() - off1 - off2 - style.ScrollbarSize;
		float maxHeight = ImGui::GetWindowHeight();

		const std::lock_guard<std::mutex> lock(Mutex);
		{
			std::vector<DisplayLogEntry*> displayedEntries;
			for (size_t i = 0; i < LogEntries.size(); i++)
			{
				DisplayLogEntry* msg = LogEntries[i];

				/* Filter level set. */
				if (this->FilterLevel != ELogLevel::ALL)
				{
					if (this->SelectedLevelOnly)
					{
						if (msg->Entry->LogLevel != this->FilterLevel)
						{
							continue;
						}
					}
					else
					{
						if (msg->Entry->LogLevel > this->FilterLevel)
						{
							continue;
						}
					}
				}

				/* additional channel filtering */
				if (activeChannels.size() == 0)
				{
					/* no channels filtered for */
					displayedEntries.push_back(msg);
				}
				else if (std::find(activeChannels.begin(), activeChannels.end(), msg->Entry->Channel) != activeChannels.end())
				{
					/* matching one of the active channels */
					displayedEntries.push_back(msg);
				}
			}

			size_t start = 0;

			/* Start Index */
			if (this->MaxShownCount > 0 && displayedEntries.size() > this->MaxShownCount)
			{
				start = displayedEntries.size() - this->MaxShownCount;
			}

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

	if (std::find_if(this->Channels.begin(), this->Channels.end(), [aLogEntry](LogChannel& channel){ return channel.Name == aLogEntry->Channel;}) == this->Channels.end())
	{
		this->Channels.push_back(LogChannel{false, aLogEntry->Channel });
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

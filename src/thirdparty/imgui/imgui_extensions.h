#ifndef IMGUI_EXTENSIONS_H
#define IMGUI_EXTENSIONS_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui
{
	static void TextCenteredColumn(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(fmt).x) / 2);
		TextV(fmt, args);
		va_end(args);
	}
	static void TextCenteredColumnV(const char* fmt, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		TextEx(g.TempBuffer, text_end, ImGuiTextFlags_NoWidthForLargeClippedText);
	}

	static bool CheckboxCenteredColumn(const char* label, bool* v)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::GetFontSize()) / 2);
		return ImGui::Checkbox(label, v);
	}

	static bool Tooltip()
	{
		bool hovered = ImGui::IsItemHovered();
		if (hovered)
		{
			ImGui::BeginTooltip();
		}
		return hovered;
	}

	static void TooltipGeneric(const char* fmt, ...)
	{
		if (ImGui::Tooltip())
		{
			va_list args;
			va_start(args, fmt);
			ImGui::TextV(fmt, args);
			va_end(args);
			ImGui::EndTooltip();
		}
	}

	static void ToggleButton(const char* label, bool* p_toggle, const ImVec2& size_arg = ImVec2(0,0))
	{
		if (ImGui::Button(label, size_arg)) { *p_toggle = !*p_toggle; }
	}

	static void TextOutlined(const char* fmt, ...)
	{
		ImVec2 pos = GetCursorPos();

		va_list args;
		va_start(args, fmt);
		pos.x += 1;
		pos.y += 1;
		SetCursorPos(pos);
		TextColoredV(ImVec4(0, 0, 0, 255), fmt, args);
		pos.x -= 1;
		pos.y -= 1;
		SetCursorPos(pos);
		TextV(fmt, args);
		va_end(args);
	}

	static void TextDisabledOutlined(const char* fmt, ...)
	{
		ImVec2 pos = GetCursorPos();

		va_list args;
		va_start(args, fmt);
		pos.x += 1;
		pos.y += 1;
		SetCursorPos(pos);
		TextColoredV(ImVec4(0, 0, 0, 255), fmt, args);
		pos.x -= 1;
		pos.y -= 1;
		SetCursorPos(pos);
		TextDisabledV(fmt, args);
		va_end(args);
	}

	static void TextColoredOutlined(const ImVec4& col, const char* fmt, ...)
	{
		ImVec2 pos = GetCursorPos();

		va_list args;
		va_start(args, fmt);
		pos.x += 1;
		pos.y += 1;
		SetCursorPos(pos);
		TextColoredV(ImVec4(0, 0, 0, 255), fmt, args);
		pos.x -= 1;
		pos.y -= 1;
		SetCursorPos(pos);
		TextColoredV(col, fmt, args);
		va_end(args);
	}

	static void TextWrappedOutlined(const char* fmt, ...)
	{
		ImVec2 pos = GetCursorPos();

		va_list args;
		va_start(args, fmt);
		pos.x += 1;
		pos.y += 1;
		SetCursorPos(pos);
		PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
		TextWrappedV(fmt, args);
		ImGui::PopStyleColor();
		pos.x -= 1;
		pos.y -= 1;
		SetCursorPos(pos);
		TextWrappedV(fmt, args);
		va_end(args);
	}

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static void AddUnderLine(ImColor col_)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
	}

	static bool TextURL(const char* name_, bool SameLineBefore_, bool SameLineAfter_)
	{
		bool clicked = false;

		if (true == SameLineBefore_) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::Text(name_);
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
				clicked = true;
			}
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		}
		else
		{
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
		if (true == SameLineAfter_) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }

		return clicked;
	}

	static bool IconButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
	{
		bool ret = false;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.863f, 0.863f, 0.863f, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });

		if (ImGui::ImageButton(user_texture_id, size, uv0, uv1, frame_padding, bg_col, tint_col))
		{
			ret = true;
		}
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar();

		return ret;
	}

	static ImVector<ImRect> s_GroupPanelLabelStack;

	static void BeginGroupPanel(const char* name, const ImVec2& size)
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvailWidth();
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Text(name);
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	static void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = s_GroupPanelLabelStack.back();
		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
		ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
				case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
					// right half-plane
				case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
					// top
				case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
					// bottom
				case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}

	namespace GW2
	{
		static bool Button(const char* label, const ImVec2& size_arg = ImVec2(0, 0))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.839f, 0.807f, 0.741f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
			bool ret = ButtonEx(label, size_arg, ImGuiButtonFlags_None);
			ImGui::PopStyleColor(6);
			ImGui::PopStyleVar(1);

			return ret;
		}

		static bool ButtonDisabled(const char* label, const ImVec2& size_arg = ImVec2(0, 0))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
			bool ret = ButtonEx(label, size_arg, ImGuiButtonFlags_None);
			ImGui::PopStyleColor(6);
			ImGui::PopStyleVar(1);

			return ret;
		}

		static bool ContextMenuItem(const char* id, const char* label, ImTextureID bullet_texture_id, ImTextureID highlight_texture_id, const ImVec2& size_arg = ImVec2(0, 0))
		{
			float itemWidth = ImGui::GetWindowContentRegionWidth();

			float height = ImGui::GetTextLineHeight();
			ImVec2 initialPos = ImGui::GetCursorPos();

			PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			bool btn = ImGui::Button((std::string{"##Button_"} + label).c_str(), size_arg);
			bool hov = ImGui::IsItemHovered();
			ImGui::PopStyleVar();

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::SetCursorPos(ImVec2(initialPos.x + height, initialPos.y));
			ImGui::TextOutlined(label);

			ImGui::SetCursorPos(initialPos);
			if (bullet_texture_id) { ImGui::Image(bullet_texture_id, ImVec2(height, height)); }
			if (hov)
			{
				ImGui::SetCursorPos(initialPos);
				if (highlight_texture_id) { ImGui::Image(highlight_texture_id, ImVec2(itemWidth, height * 1.2f)); }
			}
			ImGui::PopItemFlag();

			ImGui::SetCursorPos(ImVec2(initialPos.x, initialPos.y + (height * 1.4f)));

			return btn;
		}

		static void TooltipGeneric(const char* fmt, ...)
		{
			/* FIXME: this style is obviously not gw2, just a simplified standard style */

			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1));
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.141f, 0.180f, 0.196f, 0.784f));
			ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				va_list args;
				va_start(args, fmt);
				ImGui::TextV(fmt, args);
				va_end(args);
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(2);
		}
	}

	static ImVec4 HEXtoIV4(const char* hex)
	{
		int r, g, b;
		int in = std::sscanf(hex, "%02x%02x%02x", &r, &g, &b);
		return in == 0 ? ImVec4() : ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	}
}

#endif
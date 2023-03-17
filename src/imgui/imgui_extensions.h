#ifndef IMGUI_EXTENSIONS_H
#define IMGUI_EXTENSIONS_H

#include "imgui.h"
#include "imgui_internal.h"

#include "../core.h"

namespace ImGui
{
	/* Limitations: DO NOT USE in combination with printf style formating, use only single wide char arguments. */
	static void TextW(const wchar_t* fmt, ...)
	{
		std::wstring fmtStrW = fmt;
		std::string fmtStr = WStrToStr(fmtStrW);
		const char* str = fmtStr.c_str();

		va_list args;
		va_start(args, str);
		TextV(str, args);
		va_end(args);
	}

	/* Limitations: DO NOT USE in combination with printf style formating, use only single wide char arguments. */
	static void TextWrappedW(const wchar_t* fmt, ...)
	{
		std::wstring fmtStrW = fmt;
		std::string fmtStr = WStrToStr(fmtStrW);
		const char* str = fmtStr.c_str();

		va_list args;
		va_start(args, str);
		TextWrappedV(str, args);
		va_end(args);
	}

	/* Limitations: DO NOT USE in combination with printf style formating, use only single wide char arguments. */
	static void TextDisabledW(const wchar_t* fmt, ...)
	{
		std::wstring fmtStrW = fmt;
		std::string fmtStr = WStrToStr(fmtStrW);
		const char* str = fmtStr.c_str();

		va_list args;
		va_start(args, str);
		TextDisabledV(str, args);
		va_end(args);
	}

	static bool TreeNodeW(const wchar_t* label)
	{
		std::wstring fmtStrW = label;
		std::string fmtStr = WStrToStr(fmtStrW);
		const char* str = fmtStr.c_str();

		return TreeNode(str);
	}

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
	static void TextCenteredColumnW(const wchar_t* label)
	{
		std::wstring fmtStrW = label;
		std::string fmtStr = WStrToStr(fmtStrW);
		const char* str = fmtStr.c_str();

		TextCenteredColumn(str);
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
			ImGui::Text(fmt);
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
}

#endif
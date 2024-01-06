#include "MenuItem.h"

#include "Renderer.h"

#include "Menu.h"
#include "Textures/Texture.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	namespace Menu
	{
		bool MenuItem::Render()
		{
			bool result = false;

			float wndWidth = ImGui::GetWindowContentRegionWidth();

			float width = 190 * Renderer::Scaling;
			float height = 26 * Renderer::Scaling;

			if (Menu::MenuButton && Menu::MenuButtonHover)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));

				ImVec2 pos = ImGui::GetCursorPos();
				ImVec2 offset = ImGui::CalcTextSize(Label.c_str());

				if (Icon != nullptr && Icon->Resource != nullptr)
				{
					float iconSize = 32 * Renderer::Scaling;
					if (IsHovering)
					{
						iconSize *= 1.1f;
					}

					ImVec2 wndPos = ImGui::GetWindowPos();
					ImVec2 posIconTL = pos;
					posIconTL.x += wndPos.x + (width * 0.1f);
					posIconTL.y += wndPos.y + ((height - iconSize) / 2.0f);
					ImVec2 posIconBR = posIconTL;
					// offset.y is text height, use as a reference
					posIconBR.x += iconSize;
					posIconBR.y += iconSize;

					ImGui::GetForegroundDrawList(ImGui::GetCurrentWindow())->AddImage(Icon->Resource, posIconTL, posIconBR, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
				}
				
				pos.x = (wndWidth - width) / 2.0f + 8.0f;

				ImVec2 txtPos = pos;
				txtPos.x += width * 0.25f;
				txtPos.y += (height - offset.y) / 2.0f;

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
				ImGui::SetCursorPos(pos);
				if (ImGui::ImageButton(!IsHovering ? Menu::MenuButton->Resource : Menu::MenuButtonHover->Resource, ImVec2(width, height)))
				{
					*(Toggle) = !*(Toggle);
					result = *(Toggle);
				}
				ImGui::PopStyleVar();
				IsHovering = ImGui::IsItemHovered();

				ImGui::SetCursorPos(txtPos);
				ImGui::Text(Label.c_str());

				ImGui::PopStyleColor(3);

				// advance cursor pos
				ImGui::SetCursorPosY(pos.y + height + 6.0f);
			}

			return result;
		}
	}
}
#include "MenuItem.h"

#include "Renderer.h"
#include "Shared.h"

#include "Menu.h"
#include "Textures/TextureLoader.h"
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
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

				ImVec2 pos = ImGui::GetCursorPos();
				ImVec2 offset = ImGui::CalcTextSize(Label.c_str());

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

				if (Icon != nullptr && Icon->Resource != nullptr)
				{
					float iconSize = 32.0f * Renderer::Scaling;
					if (IsHovering)
					{
						iconSize *= 1.1f;
					}

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::SetCursorPos(ImVec2(pos.x, pos.y - 4.0f));
					ImGui::Image(Icon->Resource, ImVec2(iconSize, iconSize));
					ImGui::PopItemFlag();
				}
				else
				{
					Icon = TextureLoader::GetOrCreate(TextureIdentifier.c_str(), ResourceID, NexusHandle);
				}

				ImGui::SetCursorPos(txtPos);
				ImGui::Text(Label.c_str());

				ImGui::PopStyleColor(4);

				// advance cursor pos
				ImGui::SetCursorPosY(pos.y + height + 6.0f);
			}

			return result;
		}
	}
}
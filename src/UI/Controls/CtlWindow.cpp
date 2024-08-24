///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlWindow.cpp
/// Description  :  Contains the functionality for a window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CtlWindow.h"

#include "imgui/imgui.h"

void IWindow::Render()
{
	if (this->IsPopOut())
	{
		this->RenderBegin();
		this->RenderContent();
		this->RenderEnd();
	}
	else
	{
		this->RenderContent();
	}
}

const std::string& IWindow::GetName()
{
	return this->Name;
}

bool IWindow::IsVisible()
{
	return this->Visible;
}

bool IWindow::IsPopOut()
{
	return this->PoppedOut;
}

void IWindow::RenderBegin()
{
	ImGui::Begin(this->Name.c_str(), &this->Visible, ImGuiWindowFlags_NoCollapse);
}

void IWindow::RenderEnd()
{
	float height = ImGui::GetWindowHeight();
	float width = ImGui::GetWindowWidth();

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize();

	ImVec2 btnPos = ImVec2(width - style.WindowPadding.x - btnSz, height - style.WindowPadding.y - btnSz);

	ImGui::SetCursorPos(btnPos);
	if (ImGui::Button(("((PLACEHOLDER))##" + this->Name).c_str(), ImVec2(btnSz, btnSz)))
	{
		this->PoppedOut = true;
	}

	ImGui::End();
}

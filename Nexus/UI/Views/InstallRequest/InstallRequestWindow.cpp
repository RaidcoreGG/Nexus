///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  InstallRequestWindow.cpp
/// Description  :  Window listing addon installs requested through nexus:// links.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#include "InstallRequestWindow.h"

#include <string>
#include <vector>

#include "Host/Library/LibAddon.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_internal.h"
#include "thirdparty/Clockwork/Clockwork.h"
#include "thirdparty/Clockwork/Tasks/CancellationToken.h"
#include "thirdparty/Clockwork/Tasks/ETaskPriority.h"

#include "Runtime/Runtime.h"
#include "Util/Url.h"

namespace Raidcore::Nexus::GUI
{
	void CInstallRequestWindow::Render()
	{
		Runtime& ctx = Runtime::Get();
		Host::UrlRequestMgr& urlRequests = ctx.UrlRequests();

		std::vector<Host::LibraryAddon_t> pending = urlRequests.GetPending();
		
		// delete selected state for non existent pending
		std::erase_if(this->Selected, [&pending](const auto& aEntry)
		{
			return std::ranges::find(pending, aEntry.first, &Host::LibraryAddon_t::Signature) == pending.end();
		});

		if (pending.empty()) { return; }

		Localization* lang = ctx.UI().GetLocalization();
		Graphics::Window_t& grWindow = ctx.GrWindow();

		constexpr ImGuiWindowFlags flags
			= ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoSavedSettings;

		ImGui::SetNextWindowPos(ImVec2(grWindow.Width * 0.5f, grWindow.Height * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (!ImGui::Begin(lang->Translate("((Install requests))##InstallRequests"), nullptr, flags))
		{
			ImGui::End();
			return;
		}

		ImGui::TextUnformatted(lang->Translate("((Requested installs from url handler:))"));

		ImGui::Separator();

		for (const Host::LibraryAddon_t& addon : pending)
		{
			// try_emplace keeps existing state, new requests start checked
			bool& selected = this->Selected.try_emplace(addon.Signature, true).first->second;

			ImGui::PushID(static_cast<int>(addon.Signature));

			ImGui::Checkbox("##Selected", &selected);
			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Text("%s", addon.Name.c_str());
			ImGui::SameLine();
			ImGui::TextDisabled("%s %s", lang->Translate("((by))"), addon.Author.c_str());
			ImGui::TextDisabled("%s %d | %s", lang->Translate("((Policy tier:))"), addon.PolicyTier, URL::GetBase(addon.DownloadURL).c_str());
			ImGui::EndGroup();

			if (ImGui::IsItemHovered() && !addon.Description.empty())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
				ImGui::TextUnformatted(addon.Description.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			ImGui::PopID();
		}

		ImGui::Separator();

		bool anySelected = std::ranges::any_of(pending, [this](const Host::LibraryAddon_t& aAddon)
		{
			return this->Selected[aAddon.Signature];
		});

		if (!anySelected)
		{
			// disabled
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button(lang->Translate("((000028))")))
		{
			for (const Host::LibraryAddon_t& addon : pending)
			{
				urlRequests.RemovePending(addon.Signature);
			}

			std::erase_if(pending, [this](const Host::LibraryAddon_t& addon)
			{
				return !this->Selected[addon.Signature];
			});

			Clockwork::Run<void>(Clockwork::ETaskPriority::Normal, [pending](Clockwork::CancellationToken aToken)
			{
				for (const Host::LibraryAddon_t& addon : pending)
				{
					Runtime::Get().Library().Install(addon.Signature);
				}
			});
		}

		if (!anySelected)
		{
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}

		ImGui::SameLine();

		if (ImGui::Button(lang->Translate("((Discard))")))
		{
			for (const Host::LibraryAddon_t& addon : pending)
			{
				urlRequests.RemovePending(addon.Signature);
			}
		}

		ImGui::End();
	}
}

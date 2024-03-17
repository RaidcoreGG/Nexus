#include "CMumbleOverlay.h"

#include "Shared.h"
#include "State.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	void CMumbleOverlay::Render()
	{
		if (!Visible) { return; }

		ImVec2 pos = ImVec2(16.0f, 16.0f);
		ImVec2 size = ImVec2(ImGui::GetFontSize() * 10.0f, 0.0f);

		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		if (!State::IsMumbleDisabled)
		{
			if (ImGui::Begin("MumbleUI", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Interface");
				ImGui::Separator();
				if (ImGui::BeginTable("table_ui", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("version");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->UIVersion);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("tick");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->UITick);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mapopen");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsMapOpen ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("focus");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsGameFocused ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("pvp");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsCompetitive ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("txtfocus");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsTextboxFocused ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("combat");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsInCombat ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("cmploc");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleLink->Context.IsCompassTopRight ? "top" : "bottom");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("cmpwidth");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.Compass.Width);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("cmpheight");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.Compass.Height);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("cmprot");
					ImGui::TableSetColumnIndex(1);
					MumbleLink->Context.IsCompassRotating ? ImGui::Text("%+1.4f", MumbleLink->Context.Compass.Rotation) : ImGui::Text("%s", "disabled");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mapx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%09.4f", MumbleLink->Context.Compass.Center.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mapy");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%09.4f", MumbleLink->Context.Compass.Center.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mapscale");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%1.4f", MumbleLink->Context.Compass.Scale);

					ImGui::EndTable();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.35f);
			pos.x += size.x + 16.0f;
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			if (ImGui::Begin("MumblePlayer", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Player");
				ImGui::Separator();
				if (ImGui::BeginTable("table_player", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarPosition.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posy");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarPosition.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posz");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarPosition.Z);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("frontx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarFront.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("fronty");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarFront.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("frontz");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->AvatarFront.Z);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("gx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%09.4f", MumbleLink->Context.Compass.PlayerPosition.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("gy");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%09.4f", MumbleLink->Context.Compass.PlayerPosition.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mount");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleLink->Context.MountIndex);

					ImGui::EndTable();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.35f);
			pos.x += size.x + 16.0f;
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			if (ImGui::Begin("MumbleCamera", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Camera");
				ImGui::Separator();
				if (ImGui::BeginTable("table_camera", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraPosition.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posy");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraPosition.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("posz");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraPosition.Z);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("frontx");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraFront.X);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("fronty");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraFront.Y);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("frontz");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%+09.4f", MumbleLink->CameraFront.Z);

					ImGui::EndTable();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.35f);
			pos.x += size.x + 16.0f;
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			if (ImGui::Begin("MumbleGame", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Game State");
				ImGui::Separator();
				if (ImGui::BeginTable("table_game", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("mapid");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.MapID);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("maptype");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleLink->Context.MapType);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("ip");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u.%u.%u.%u",
						MumbleLink->Context.ServerAddress[4],
						MumbleLink->Context.ServerAddress[5],
						MumbleLink->Context.ServerAddress[6],
						MumbleLink->Context.ServerAddress[7]);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("shard");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.ShardID);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("instance");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.InstanceID);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("build");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.BuildID);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("pid");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleLink->Context.ProcessID);

					ImGui::EndTable();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.35f);
			pos.x += size.x + 16.0f;
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			if (ImGui::Begin("MumbleIdentity", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Identity");
				ImGui::Separator();
				if (MumbleIdentity != nullptr)
				{
					if (ImGui::BeginTable("table_identity", 2))
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("cmdr");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%s", MumbleIdentity->IsCommander ? "true" : "false");
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("fov");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%1.4f", MumbleIdentity->FOV);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("name");
						ImGui::TableSetColumnIndex(1); ImGui::Text(&MumbleIdentity->Name[0]);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("prof");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleIdentity->Profession);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("race");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleIdentity->Race);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("spec");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleIdentity->Specialization);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("team");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleIdentity->TeamColorID);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("uisz");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%d", MumbleIdentity->UISize);
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::Text("world");
						ImGui::TableSetColumnIndex(1); ImGui::Text("%u", MumbleIdentity->WorldID);

						ImGui::EndTable();
					}
				}
				else
				{
					ImGui::Text("Identity unitialised.");
				}
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.35f);
			pos.x += size.x + 16.0f;
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			if (ImGui::Begin("NexusLinkData", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Nexus Mumble Extensions");
				ImGui::Separator();
				if (ImGui::BeginTable("table_nexus", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("frame");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%d", FrameCounter);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("isMoving");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", IsMoving ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("isCameraMoving");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", IsCameraMoving ? "true" : "false");
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("isGameplay");
					ImGui::TableSetColumnIndex(1); ImGui::Text("%s", IsGameplay ? "true" : "false");

					ImGui::EndTable();
				}
			}
			ImGui::End();
		}
		else
		{
			if (ImGui::Begin("MumbleDisabled", &Visible, WindowFlags_Overlay))
			{
				ImGui::Text("Mumble API disabled.");
			}
			ImGui::End();
		}
	}
}
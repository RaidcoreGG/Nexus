#include "QuickAccess.h"

#include "Shared.h"
#include "Index.h"
#include "State.h"
#include "Renderer.h"
#include "Consts.h"

#include "Services/Textures/Texture.h"
#include "Services/Textures/TextureLoader.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"

#include "resource.h"

#include "GUI/GUI.h"

namespace GUI
{
	namespace QuickAccess
	{
#define GW2_QUICKACCESS_ITEMS 10;

		float size = 32.0f;

		float Opacity = 0.5f;

		std::mutex								Mutex;
		std::map<std::string, Shortcut>			Registry;
		std::map<std::string, SimpleShortcut>	OrphanedCallbacks;

		std::thread		AnimationThread;
		bool			IsAnimating			= false;
		bool			IsFadingIn			= false;
		bool			IsHovering			= false;

		bool			VerticalLayout		= false;
		EQAVisibility	Visibility			= EQAVisibility::AlwaysShow;
		EQAPosition		Location			= EQAPosition::Extend;
		ImVec2			Offset				= ImVec2(0, 0);

		Texture*		IconNotification	= nullptr;

		void Fade()
		{
			IsAnimating = true;
			while (IsAnimating)
			{
				if (IsFadingIn)				{ Opacity += 0.05f; }
				else						{ Opacity -= 0.05f; }

				if (Opacity > 1)			{ Opacity = 1.0f; IsAnimating = false; }
				else if (Opacity < 0.5f)	{ Opacity = 0.5f; IsAnimating = false; }

				Sleep(35);
			}
		}

		void Render()
		{
			switch (Visibility)
			{
				/* continue rendering */
				default:
				case EQAVisibility::AlwaysShow: { break; }

				case EQAVisibility::Gameplay:
				{
					/* don't render if not gameplay */
					if (!GUI::NexusLink->IsGameplay)
					{
						return;
					}

					break;
				}
				case EQAVisibility::OutOfCombat:
				{
					/* don't render if not gameplay */
					if (!GUI::NexusLink->IsGameplay)
					{
						return;
					}

					/* don't render if in combat */
					if (GUI::MumbleLink->Context.IsInCombat)
					{
						return;
					}

					break;
				}
				case EQAVisibility::InCombat:
				{
					/* don't render if not gameplay */
					if (!GUI::NexusLink->IsGameplay)
					{
						return;
					}

					/* don't render if out of combat */
					if (!GUI::MumbleLink->Context.IsInCombat)
					{
						return;
					}

					break;
				}

				/* don't render*/
				case EQAVisibility::Hide: { return; }
			}

			bool isActive = false;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Opacity);

			ImVec2 pos = ImVec2(0.0f, 0.0f);

			switch (Location)
			{
				case EQAPosition::Extend:
					pos.x += (size * Renderer::Scaling) * GW2_QUICKACCESS_ITEMS;
					break;
				case EQAPosition::Under:
					pos.y += size * Renderer::Scaling;
					break;
				case EQAPosition::Bottom:
					pos.y += Renderer::Height - (size * 2 * Renderer::Scaling);
					break;
			}

			pos.x += Offset.x;
			pos.y += Offset.y;

			ImGui::SetNextWindowPos(pos);
			if (ImGui::Begin("QuickAccessBar", (bool*)0, WindowFlags_Custom))
			{
				bool menuFound = false;

				unsigned c = 0;
				const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);
				for (auto& [identifier, shortcut] : Registry)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));

					//Logger->Debug(CH_QUICKACCESS, "size: %f | c: %d | scale: %f", size, c, Renderer::Scaling);

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
					if (VerticalLayout)
					{
						ImGui::SetCursorPos(ImVec2(0, ((size * c) + (c ? 1 : 0)) * Renderer::Scaling));
					}
					else
					{
						ImGui::SetCursorPos(ImVec2(((size * c) + (c ? 1 : 0)) * Renderer::Scaling, 0));
					}

					ImVec2 pos = ImGui::GetCursorPos();

					bool iconHovered = false;

					if (shortcut.TextureNormal && shortcut.TextureNormal->Resource &&
						shortcut.TextureHover && shortcut.TextureHover->Resource)
					{
						if (ImGui::ImageButton(!shortcut.IsHovering ? shortcut.TextureNormal->Resource : shortcut.TextureHover->Resource, ImVec2(size * Renderer::Scaling, size * Renderer::Scaling)))
						{
							isActive = true;
							if (shortcut.InputBind.length() > 0)
							{
								shortcut.HasNotification = false;
								InputBindApi->Invoke(shortcut.InputBind);
							}
						}
						iconHovered = ImGui::IsItemHovered();
					}
					else if (shortcut.TextureGetAttempts < 10)
					{
						shortcut.TextureNormal = TextureService->Get(shortcut.TextureNormalIdentifier.c_str());
						shortcut.TextureHover = TextureService->Get(shortcut.TextureHoverIdentifier.c_str());
						shortcut.TextureGetAttempts++;
					}
					else
					{
						Logger->Warning(CH_QUICKACCESS, "Cancelled getting textures for shortcut \"%s\" after 10 failed attempts.", identifier.c_str());

						/* fallback icons */
						shortcut.TextureNormal = TextureService->Get(ICON_GENERIC);
						shortcut.TextureHover = TextureService->Get(ICON_GENERIC_HOVER);

						/* absolute sanity check */
						if (shortcut.TextureNormal == nullptr || shortcut.TextureHover == nullptr)
						{
							Logger->Warning(CH_QUICKACCESS, "Neither promised textures nor fallback textures could be loaded, removing shortcut \"%s\".", identifier.c_str());

							// call this async because we're currently iterating the list
							std::thread([identifier]()
								{
									RemoveShortcut(identifier.c_str());
								})
								.detach();
						}
					}

					bool notifHovered = false;

					if (IconNotification && IconNotification->Resource)
					{
						if (shortcut.HasNotification)
						{
							float offIcon = (size * Renderer::Scaling) / 2.0f;

							pos.x += offIcon;
							pos.y += offIcon;

							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::SetCursorPos(pos);
							ImGui::Image(IconNotification->Resource, ImVec2(offIcon, offIcon));
							ImGui::PopItemFlag();
						}
						notifHovered = ImGui::IsItemHovered();
					}
					else
					{
						IconNotification = TextureService->GetOrCreate(ICON_NOTIFICATION, RES_ICON_NOTIFICATION, NexusHandle);
					}

					ImGui::PopStyleVar();
					shortcut.IsHovering = iconHovered || notifHovered;
					if (shortcut.TooltipText.length() > 0)
					{
						ImGui::TooltipGeneric(Language->Translate(shortcut.TooltipText.c_str()));
					}

					ImGui::PopStyleColor(3);

					c++;

					RenderContextMenu(identifier, shortcut, &isActive);
				}

				bool isHoveringNative = false;
				if (Location == EQAPosition::Extend)
				{
					ImVec2 mPos = ImGui::GetMousePos();
					if (mPos.x != -FLT_MAX && mPos.y != -FLT_MAX && mPos.x < pos.x - Offset.x && mPos.y < Renderer::Scaling * size)
					{
						isHoveringNative = true;
					}
				}

				bool newHoverState = ImGui::IsWindowHovered() || isActive || isHoveringNative;
				if (newHoverState != IsHovering)
				{
					if (newHoverState) { IsFadingIn = true; }
					else { IsFadingIn = false; }

					if (!IsAnimating)
					{
						AnimationThread = std::thread(Fade);
						AnimationThread.detach();
					}
				}
				IsHovering = newHoverState;

			}
			ImGui::End();

			ImGui::PopStyleVar();
		}
		void RenderContextMenu(const std::string& aIdentifier, const Shortcut& aShortcut, bool* aIsActive)
		{
			if (aShortcut.ContextItems.size() > 0)
			{
				std::string ctxId = "ShortcutsCtxMenu##" + aIdentifier;
				if (ImGui::BeginPopupContextItem(ctxId.c_str()))
				{
					*aIsActive = true;

					int amtItems = static_cast<int>(aShortcut.ContextItems.size());
					int idx = 0;

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
					for (auto& [cbidentifier, cbshortcut] : aShortcut.ContextItems)
					{
						idx++;

						if (cbshortcut.Callback)
						{
							ImGui::TextDisabled(Loader::GetOwner(cbshortcut.Callback).c_str());
							cbshortcut.Callback();

							if (idx != amtItems)
							{
								ImGui::Separator();
							}
						}
					}
					ImGui::PopStyleVar();

					ImGui::EndPopup();
				}
				ImGui::OpenPopupOnItemClick(ctxId.c_str(), 1);
			}
		}

		void WhereAreMyParents()
		{
			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);

			std::vector<std::string> remove;

			for (auto& [identifier, shortcut] : OrphanedCallbacks)
			{
				auto it = Registry.find(shortcut.TargetShortcut);
				if (it != Registry.end())
				{
					it->second.ContextItems[identifier] = shortcut;
				}
			}

			/* no longer orphans */
			for (std::string remidentifier : remove)
			{
				OrphanedCallbacks.erase(remidentifier);
			}
		}

		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText)
		{
			std::string str = aIdentifier;
			std::string strTexId = aTextureIdentifier;
			std::string strTexHoverId = aTextureHoverIdentifier;
			std::string strKbId = aInputBindIdentifier;
			std::string strTT = aTooltipText;

			{
				const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);
				if (Registry.find(str) == Registry.end())
				{
					Texture* normal = TextureService->Get(strTexId.c_str());
					Texture* hover = TextureService->Get(strTexHoverId.c_str());
					Shortcut sh{};
					sh.TextureNormalIdentifier = aTextureIdentifier;
					sh.TextureHoverIdentifier = aTextureHoverIdentifier;
					sh.TextureNormal = normal;
					sh.TextureHover = hover;
					sh.InputBind = aInputBindIdentifier;
					sh.TooltipText = aTooltipText;
					sh.TextureGetAttempts = 0;
					Registry[str] = sh;

					int amt = 0;
					if (sh.TextureNormal != nullptr) { amt++; }
					if (sh.TextureHover != nullptr) { amt++; }

					/*if (amt < 2)
					{
						Logger->Debug(CH_QUICKACCESS, "Shortcut \"%s\" was promised 2 textures, but received %d.", str.c_str(), amt);
					}*/
				}
			}

			WhereAreMyParents();
		}
		void RemoveShortcut(const char* aIdentifier)
		{
			std::string str = aIdentifier;

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);

			auto it = Registry.find(str);
			if (it != Registry.end())
			{
				for (auto& orphan : it->second.ContextItems)
				{
					OrphanedCallbacks[orphan.first] = orphan.second;
				}
			}
			Registry.erase(str);

		}
		void NotifyShortcut(const char* aIdentifier)
		{
			std::string str = aIdentifier;

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);
			
			auto it = Registry.find(str);

			if (it != Registry.end())
			{
				it->second.HasNotification = true;
			}
		}
		void SetNotificationShortcut(const char* aIdentifier, bool aState)
		{
			std::string str = aIdentifier;

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);
			
			auto it = Registry.find(str);

			if (it != Registry.end())
			{
				it->second.HasNotification = aState;
			}
		}

		void AddSimpleShortcut(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback)
		{
			AddSimpleShortcut2(aIdentifier, QA_MENU, aShortcutRenderCallback);
		}
		void AddSimpleShortcut2(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback)
		{
			std::string str = aIdentifier;
			std::string tarShStr = aTargetShortcutIdentifier ? aTargetShortcutIdentifier : QA_MENU;

			SimpleShortcut shortcut{
				aIdentifier,
				aShortcutRenderCallback
			};

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);

			auto it = Registry.find(tarShStr);
			if (it != Registry.end())
			{
				it->second.ContextItems[str] = shortcut;
			}
			else
			{
				OrphanedCallbacks[str] = shortcut;
			}
		}
		void RemoveSimpleShortcut(const char* aIdentifier)
		{
			std::string str = aIdentifier;

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);

			for (auto& [identifier, shortcut] : Registry)
			{
				shortcut.ContextItems.erase(aIdentifier);
			}

			OrphanedCallbacks.erase(aIdentifier);
		}

		std::string EQAVisibilityToString(EQAVisibility aQAVisibility)
		{
			switch (aQAVisibility)
			{
				case EQAVisibility::AlwaysShow:
					return "((000047))";
				case EQAVisibility::Gameplay:
					return "((000093))";
				case EQAVisibility::OutOfCombat:
					return "((000094))";
				case EQAVisibility::InCombat:
					return "((000095))";
				case EQAVisibility::Hide:
					return "((000096))";
				default:
					return NULLSTR;
			}
		}
		std::string EQAPositionToString(EQAPosition aQAPosition)
		{
			switch (aQAPosition)
			{
				case EQAPosition::Extend:
					return "((000067))";
				case EQAPosition::Under:
					return "((000068))";
				case EQAPosition::Bottom:
					return "((000069))";
				case EQAPosition::Custom:
					return "((000070))";
				default:
					return NULLSTR;
			}
		}

		int Verify(void* aStartAddress, void* aEndAddress)
		{
			int refCounter = 0;

			const std::lock_guard<std::mutex> lock(QuickAccess::Mutex);
			
			/* remove active callbacks */
			for (auto& [identifier, shortcut] : Registry)
			{
				std::vector<std::string> remove;

				for (auto& [cbidentifier, cbshortcut] : shortcut.ContextItems)
				{
					if (cbshortcut.Callback >= aStartAddress && cbshortcut.Callback <= aEndAddress)
					{
						remove.push_back(cbidentifier);
						refCounter++;
					}
				}

				for (std::string remidentifier : remove)
				{
					shortcut.ContextItems.erase(remidentifier);
				}
			}

			/* remove bastard children */
			std::vector<std::string> remove;

			for (auto& [cbidentifier, cbshortcut] : OrphanedCallbacks)
			{
				if (cbshortcut.Callback >= aStartAddress && cbshortcut.Callback <= aEndAddress)
				{
					remove.push_back(cbidentifier);
					refCounter++;
				}
			}

			for (std::string remidentifier : remove)
			{
				OrphanedCallbacks.erase(remidentifier);
			}

			return refCounter;
		}
	}
}
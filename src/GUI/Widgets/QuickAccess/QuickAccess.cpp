#include "QuickAccess.h"

namespace GUI
{
	namespace QuickAccess
	{
		float size = 32.0f;

		float Opacity = 0.5f;

		std::mutex													Mutex;
		std::map<std::string, Shortcut>								Registry;
		std::map<std::string, QUICKACCESS_SHORTCUTRENDERCALLBACK>	RegistrySimple;

		std::thread		AnimationThread;
		bool			IsAnimating			= false;
		bool			IsFadingIn			= false;
		bool			IsHovering			= false;

		bool			VerticalLayout		= false;
		EQAPosition		Location			= EQAPosition::Extend;
		ImVec2			Offset				= ImVec2(((size + 1) * Renderer::Scaling) * 9, 0.0f);

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
			bool isActive = false;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Opacity);

			ImVec2 pos = ImVec2(0.0f, 0.0f);

			switch (Location)
			{
				case EQAPosition::Extend:
					pos.x += (size * Renderer::Scaling) * 9;
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
			if (ImGui::Begin("QuickAccessBar", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar))
			{
				bool menuFound = false;

				unsigned c = 0;
				Mutex.lock();
				{

					for (auto& [identifier, shortcut] : Registry)
					{
						if (shortcut.TextureNormal && shortcut.TextureHover)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));

							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
							if (VerticalLayout)
							{
								ImGui::SetCursorPos(ImVec2(0, ((size * c) + c ? 1 : 0) * Renderer::Scaling));
							}
							else
							{
								ImGui::SetCursorPos(ImVec2(((size * c) + c ? 1 : 0) * Renderer::Scaling, 0));
							}
							if (ImGui::ImageButton(!shortcut.IsHovering ? shortcut.TextureNormal->Resource : shortcut.TextureHover->Resource, ImVec2(size * Renderer::Scaling, size * Renderer::Scaling)))
							{
								isActive = true;
								if (shortcut.Keybind.length() > 0)
								{
									Keybinds::Invoke(shortcut.Keybind);
								}
							}
							ImGui::PopStyleVar();
							shortcut.IsHovering = ImGui::IsItemHovered();
							if (shortcut.TooltipText.length() > 0)
							{
								ImGui::TooltipGeneric(shortcut.TooltipText.c_str());
							}

							ImGui::PopStyleColor(3);

							c++;

							if (!menuFound)
							{
								if (identifier == QA_MENU)
								{
									menuFound = true; // simple optimization
									if (ImGui::BeginPopupContextItem("ShortcutsCtxMenu"))
									{
										isActive = true;
										if (RegistrySimple.size() == 0)
										{
											ImGui::TextDisabled("No shortcuts added.");
										}
										else
										{
											ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
											for (auto& [identifier, shortcut] : RegistrySimple)
											{
												if (shortcut) { shortcut(); }
											}
											ImGui::PopStyleVar();
										}
										ImGui::EndPopup();
									}
									ImGui::OpenPopupOnItemClick("ShortcutsCtxMenu", 1);
								}
							}
						}
					}
				}
				Mutex.unlock();

				bool newHoverState = ImGui::IsWindowHovered() || isActive;
				if (newHoverState != IsHovering)
				{
					if (newHoverState)	{ IsFadingIn = true; }
					else				{ IsFadingIn = false; }

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

		void AddShortcut(std::string aIdentifier, std::string aTextureIdentifier, std::string aTextureHoverIdentifier, std::string aKeybindIdentifier, std::string aTooltipText)
		{
			Mutex.lock();
			{
				if (Registry.find(aIdentifier) == Registry.end())
				{
					Texture* normal = TextureLoader::Get(aTextureIdentifier);
					Texture* hover = TextureLoader::Get(aTextureHoverIdentifier);
					Shortcut sh{};
					sh.TextureNormal = normal;
					sh.TextureHover = hover;
					sh.Keybind = aKeybindIdentifier;
					sh.TooltipText = aTooltipText;
					Registry[aIdentifier] = sh;

					// keep trying to get the texture for a little bit
					if (normal == nullptr || hover == nullptr)
					{
						std::thread([aIdentifier, aTextureIdentifier, aTextureHoverIdentifier]()
						{
							int tries = 0;
							int amt = 0;
							if (Registry[aIdentifier].TextureNormal != nullptr) { amt++; }
							if (Registry[aIdentifier].TextureHover != nullptr) { amt++; }

							LogDebug(CH_QUICKACCESS, "Shortcut \"%s\" was promised 2 textures, but received %d.", aIdentifier.c_str(), amt);
							Sleep(1000); // first retry after 1s

							while (Registry[aIdentifier].TextureNormal == nullptr || Registry[aIdentifier].TextureHover == nullptr)
							{
								if (tries > 10)
								{
									LogWarning(CH_QUICKACCESS, "Cancelled getting textures for shortcut \"%s\" after 10 failed attempts.", aIdentifier.c_str());
									break;
								}

								if (Registry[aIdentifier].TextureNormal == nullptr) { Registry[aIdentifier].TextureNormal = TextureLoader::Get(aTextureIdentifier); }
								if (Registry[aIdentifier].TextureHover == nullptr) { Registry[aIdentifier].TextureHover = TextureLoader::Get(aTextureHoverIdentifier); }

								tries++;
								Sleep(5000);
							}

							/* if not all tries were used, then the texture was loaded */
							if (tries <= 10)
							{
								LogDebug(CH_QUICKACCESS, "Shortcut \"%s\" received promised textures after %d attempt(s).", aIdentifier.c_str(), tries);
								return;
							}

							/* fallback icons */
							Registry[aIdentifier].TextureNormal = TextureLoader::Get(ICON_GENERIC);
							Registry[aIdentifier].TextureHover = TextureLoader::Get(ICON_GENERIC_HOVER);

							/* absolute sanity check */
							if (Registry[aIdentifier].TextureNormal == nullptr || Registry[aIdentifier].TextureHover == nullptr)
							{
								LogWarning(CH_QUICKACCESS, "Neither promised textures nor fallback textures could be loaded, removing shortcut \"%s\".", aIdentifier.c_str());
								RemoveShortcut(aIdentifier);
							}
						}).detach();
					}
				}
			}
			Mutex.unlock();
		}

		void RemoveShortcut(std::string aIdentifier)
		{
			Mutex.lock();
			{
				Registry.erase(aIdentifier);
			}
			Mutex.unlock();
		}

		void AddSimpleShortcut(std::string aIdentifier, QUICKACCESS_SHORTCUTRENDERCALLBACK aShortcutRenderCallback)
		{
			Mutex.lock();
			{
				if (RegistrySimple.find(aIdentifier) == RegistrySimple.end())
				{
					RegistrySimple[aIdentifier] = aShortcutRenderCallback;
				}
			}
			Mutex.unlock();
		}

		void RemoveSimpleShortcut(std::string aIdentifier)
		{
			Mutex.lock();
			{
				RegistrySimple.erase(aIdentifier);
			}
			Mutex.unlock();
		}

		int Verify(void* aStartAddress, void* aEndAddress)
		{
			int refCounter = 0;

			Mutex.lock();
			{
				std::vector<std::string> remove;

				for (auto& [identifier, shortcutcb] : RegistrySimple)
				{
					if (shortcutcb >= aStartAddress && shortcutcb <= aEndAddress)
					{
						remove.push_back(identifier);
						refCounter++;
					}
				}
				for (std::string identifier : remove)
				{
					RegistrySimple.erase(identifier);
				}
			}
			Mutex.unlock();

			return refCounter;
		}
	}
}
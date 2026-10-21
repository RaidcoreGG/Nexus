///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiStyle.cpp
/// Description  :  Contains the implementation for UI styles.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UiStyle.h"

#include <string>

#include "imgui/imgui.h"

#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "Core/Preferences/PrefConst.h"
#include "UiContext.h"
#include "Util/Base64.h"

CUiStyle::CUiStyle()
{
	CContext* ctx = CContext::GetContext();
	this->Logger = ctx->GetLogger();
	this->Settings = ctx->GetSettingsCtx();
}

CUiStyle::~CUiStyle()
{
}

void ApplyDefaultStyle()
{
	try
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		static std::string s_NexusStyleDefault = "AACAPwAAAEEAAABBAAAAAAAAgD8AAABCAAAAQgAAAAAAAAA/AAAAAAAAAAAAAIA/AAAAAAAAgD8AAIBAAABAQAAAAAAAAAAAAAAAQQAAgEAAAIBAAACAQAAAgEAAAABAAAAAAAAAAAAAAKhBAADAQAAAYEEAABBBAAAgQQAAAAAAAIBAAACAQAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPwAAgD8AAIA/AACAPwAAgD8AAAA/AAAAPwAAAD8AAIA/j8J1PY/CdT2PwnU916NwPwAAAAAAAAAAAAAAAAAAAAAK16M9CtejPQrXoz3Xo3A/9ijcPvYo3D4AAAA/AAAAPwAAAAAAAAAAAAAAAAAAAAAK1yM+4XqUPo/C9T5xPQo/uB6FPj0KFz9I4Xo/zczMPrgehT49Chc/SOF6Px+FKz8K1yM9CtcjPQrXIz0AAIA/CtcjPuF6lD6PwvU+AACAPwAAAAAAAAAAAAAAAFyPAj8pXA8+KVwPPilcDz4AAIA/CtejPArXozwK16M8FK4HP1K4nj5SuJ4+UriePgAAgD+F69E+hevRPoXr0T4AAIA/XI8CP1yPAj9cjwI/AACAP7gehT49Chc/SOF6PwAAgD+PwnU+uB4FP65HYT8AAIA/uB6FPj0KFz9I4Xo/AACAP7gehT49Chc/SOF6P83MzD64HoU+PQoXP0jhej8AAIA/j8J1PRSuBz9I4Xo/AACAP7gehT49Chc/SOF6P1K4nj64HoU+PQoXP0jhej/NzEw/uB6FPj0KFz9I4Xo/AACAP/Yo3D72KNw+AAAAPwAAAD/NzMw9zczMPgAAQD8Urkc/zczMPc3MzD4AAEA/AACAP7gehT49Chc/SOF6P83MTD64HoU+PQoXP0jhej8fhSs/uB6FPj0KFz9I4Xo/MzNzP+tROD4yM7M+4noUPwisXD+4HoU+PQoXP0jhej/NzEw/zMxMPoTr0T57FC4/AACAP5ZDiz1e5dA9Uo0XPjXveD+UQws+3CSGPocW2T4AAIA/9igcP/YoHD/2KBw/AACAPwAAgD/2KNw+MzOzPgAAgD9mZmY/MzMzPwAAAAAAAIA/AACAP5qZGT8AAAAAAACAP1yPQj5cj0I+zcxMPgAAgD9SuJ4+UriePjMzsz4AAIA/H4VrPh+Faz4AAIA+AACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AACAPwAAgD+PwnU9uB6FPj0KFz9I4Xo/MzOzPgAAgD8AAIA/AAAAAGZmZj+4HoU+PQoXP0jhej8AAIA/AACAPwAAgD8AAIA/MzMzP83MTD/NzEw/zcxMP83MTD7NzEw/zcxMP83MTD8zM7M+";
		std::string decodeStyle = Base64::Decode(s_NexusStyleDefault, s_NexusStyleDefault.length());
		memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
	}
	catch (...)
	{
		CContext::GetContext()->GetLogger()->Debug(CH_UICONTEXT, "Error applying default style.");
	}
}

void CUiStyle::ApplyStyle(EUIStyle aStyle, std::string aValue)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	switch (aStyle)
	{
		case EUIStyle::User:
		{
			try
			{
				std::string b64_style = this->Settings->Get<std::string>(OPT_IMGUISTYLE, {});

				if (b64_style.empty())
				{
					this->ApplyStyle(EUIStyle::Nexus);
					return;
				}
				std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying user style.");
			}
			return;
		}
		case EUIStyle::Nexus:
		{
			try
			{
				std::string b64_style = "AACAPwAAoEAAAKBAAADAQAAAgD8AAMBAAACAQAAAAAAAAAA/AAAAAAAAwEAAAIA/AADAQAAAgD8AAMBAAAAAQAAAwEAAAAAAAADAQAAAgEAAAMBAAACAQAAAwEAAAIBAAAAAAAAAAAAAAABCAADAQAAAQEEAAMBAAAAAQgAAwEAAAIBAAADAQAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPwAAgD8AAIA/AACAPwAAgD+rqio/q6oqP6uqKj8AAIA/mZiYPbGwsD3h4OA97+5uP5mYmD2xsLA94eDgPYmICD+ZmJg9sbCwPeHg4D3e3V0/gYAAP4GAAD+BgAA/mpkZPwrXIz9SuB4/H4UrPwAAAABSuB4/mpkZP2ZmJj/NzEw+UrgeP5qZGT9mZiY/AABAPylcDz8pXA8/4XoUPwAAQD+ZmJg9sbCwPeHg4D0AAIA/mZiYPbGwsD3h4OA9AACAP5mYmD2xsLA94eDgPQAAgD+ZmJg9sbCwPeHg4D0AAIA/mZiYPbGwsD3h4OA9AACAPx+F6z5mZuY+16PwPhSuRz8fhSs/H4UrP9ejMD8Urkc/FK5HPxSuRz/NzEw/FK5HP83MTD/NzEw/4XpUPylcTz/NzEw/zcxMP+F6VD9SuJ4+j8J1Pc3MTD0pXI89AACAP1K4Hj+amRk/ZmYmP5qZmT5SuB4/mpkZP2ZmJj+amRk/UrgeP5qZGT9mZiY/ZmZmP+xRuD7sUbg+XI/CPjMzMz/sUbg+7FG4PlyPwj4zM7M+7FG4PuxRuD5cj8I+MzMzPwAAAD8AAAA/AAAAP5qZGT+amRk/mpkZPzMzMz8AAIA/MzMzPzMzMz9mZmY/AACAPwAAAAAAAAAAAAAAAAAAAAApXA8/KVwPP+F6FD8AAIA/j8J1Pc3MTD0pXI89AACAPzMzMz97FC4/16MwP83MzD0zMzM/exQuP9ejMD+amZk+MzMzP3sULj/XozA/9ijcPpf/kD6X/5A+4ZwRPyo6Uj+hZ7M+oWezPkLPJj+9UlY/MzMzP3sULj/D9Sg/KVwPPwAAgD4AAIA/AAAAAAAAgD8zMzM/exQuP8P1KD+PwvU+AACAPgAAgD8AAAAAAACAP3E9ij5xPYo+XI/CPgAAgD9SuJ4+UriePmZm5j4AAIA/uB6FPrgehT4pXI8+AACAPwAAAAAAAAAAAAAAAAAAAAAAAIA/AACAPwAAgD8pXI897FG4PuxRuD6uR2E/zcwMPwAAgD8AAIA/AAAAAGZmZj9mZuY+ZmbmPmZmZj/NzEw/AACAPwAAgD8AAIA/MzMzP83MTD/NzEw/zcxMP83MTD7NzEw+zcxMPs3MTD4zM7M+";
				std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying user style.");
			}
			return;
		}
		case EUIStyle::ImGui_Classic:
		{

			ApplyDefaultStyle();
			ImGui::StyleColorsClassic();
			return;
		}
		case EUIStyle::ImGui_Light:
		{
			ApplyDefaultStyle();
			ImGui::StyleColorsLight();
			return;
		}
		case EUIStyle::ImGui_Dark:
		{
			ApplyDefaultStyle();
			ImGui::StyleColorsDark();
			return;
		}
		case EUIStyle::ArcDPS_Default:
		{
			try
			{
				static std::string s_ArcStyleDefault = "AACAPwAAgEAAAIBAAAAAAAAAAAAAAKBAAABAQAAAAAAAAAA/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAIBAAACAQAAAAAAAAAAAAACgQAAAQEAAAKBAAABAQAAAQEAAAAAAAAAAAAAAAAAAAMhBAADAQAAAEEEAAAAAAADIQQAAAAAAAIBAAAAAAAAAAAAAAAAAAQAAAAAAAD8AAAA/AAAAAAAAAAAAAJhBAACYQQAAQEAAAEBAAACAPwEBAQAAAKA/zczMPw==";
				std::string decodeStyle = Base64::Decode(s_ArcStyleDefault, s_ArcStyleDefault.length());
				memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());

				static std::string s_ArcColorsDefault = "zcxMP83MTD/helQ/AACAP4/CdT4fhWs+4XqUPgAAgD+PwnU9zcxMPSlcjz0AAEA/KVyPPSlcjz3sUbg9AAAAAClcjz0pXI897FG4PZqZWT8K1yM/hesRPzMzMz/NzEw+CtcjP1K4Hj8fhSs/AAAAAFK4Hj+amRk/ZmYmP83MTD5SuB4/mpkZP2ZmJj8AAEA/KVwPPylcDz/hehQ/AABAP83MzD3sUbg9j8L1PZqZWT/NzMw97FG4PY/C9T2amVk/zczMPexRuD2PwvU9mplZP83MzD3sUbg9j8L1PTMzMz/NzMw97FG4PY/C9T2amRk/H4XrPmZm5j7Xo/A+FK5HPx+FKz8fhSs/16MwPxSuRz8Urkc/FK5HP83MTD8Urkc/zcxMP83MTD/helQ/KVxPP83MTD/NzEw/4XpUP1K4nj6PwnU9zcxMPSlcjz0AAIA/UrgeP5qZGT9mZiY/mpmZPlK4Hj+amRk/ZmYmP5qZGT9SuB4/mpkZP2ZmJj9mZmY/7FG4PuxRuD5cj8I+MzMzP+xRuD7sUbg+XI/CPjMzsz7sUbg+7FG4PlyPwj4zMzM/AAAAPwAAAD8AAAA/mpkZP5qZGT+amRk/MzMzPwAAgD8zMzM/MzMzP2ZmZj8AAIA/AAAAAAAAAAAAAAAAAAAAAClcDz8pXA8/4XoUPwAAgD+PwnU9zcxMPSlcjz0AAIA/MzMzP3sULj/XozA/zczMPTMzMz97FC4/16MwP5qZmT4zMzM/exQuP9ejMD/2KNw+l/+QPpf/kD7hnBE/KjpSP6Fnsz6hZ7M+Qs8mP71SVj8zMzM/exQuP8P1KD8pXA8/AACAPgAAgD8AAAAAAACAPzMzMz97FC4/w/UoP4/C9T4AAIA+AACAPwAAAAAAAIA/cT2KPnE9ij5cj8I+AACAP1K4nj5SuJ4+ZmbmPgAAgD+4HoU+uB6FPilcjz4AAIA/AAAAAAAAAAAAAAAAAAAAAAAAgD8AAIA/AACAPylcjz3sUbg+7FG4Pq5HYT/NzAw/AACAPwAAgD8AAAAAZmZmP2Zm5j5mZuY+ZmZmP83MTD8AAIA/AACAPwAAgD8zMzM/zcxMP83MTD/NzEw/zcxMPs3MTD7NzEw+zcxMPjMzsz4=";
				std::string decodeColors = Base64::Decode(s_ArcColorsDefault, s_ArcColorsDefault.length());
				memcpy_s(&style->Colors[0], sizeof(ImVec4) * ImGuiCol_COUNT, &decodeColors[0], decodeColors.length());
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying ArcDPS default style.");
			}
			return;
		}
		case EUIStyle::ArcDPS_Current:
		{
			std::filesystem::path arcIniPath = Index(EPath::DIR_ADDONS) / "arcdps/arcdps.ini";

			if (std::filesystem::exists(arcIniPath))
			{
				char buff[4096]{};
				std::string decode{};

				try
				{
					memset(buff, 0, sizeof(buff)); // reset buffer
					GetPrivateProfileStringA("session", "appearance_imgui_style180", "", &buff[0], sizeof(buff), arcIniPath.string().c_str());
					std::string arcstyle = buff;
					decode = Base64::Decode(arcstyle, arcstyle.length());
					memcpy(style, &decode[0], decode.length());

					memset(buff, 0, sizeof(buff)); // reset buffer
					GetPrivateProfileStringA("session", "appearance_imgui_colours180", "", &buff[0], sizeof(buff), arcIniPath.string().c_str());
					std::string arccols = buff;
					decode = Base64::Decode(arccols, arccols.length());
					memcpy(&style->Colors[0], &decode[0], decode.length());
				}
				catch (...)
				{
					this->Logger->Warning(CH_UICONTEXT, "Couldn't parse ArcDPS style.");
				}
			}
			else
			{
				this->Logger->Warning(CH_UICONTEXT, "Tried importing ArcDPS style, with no config present.");
			}
			return;
		}
		case EUIStyle::File:
		{
			std::filesystem::path path = Index(EPath::DIR_STYLES) / aValue;

			if (std::filesystem::is_directory(path)) { return; }
			if (std::filesystem::file_size(path) == 0) { return; }
			if (path.extension() != ".imstyle180") { return; }

			ImGuiStyle* style = &ImGui::GetStyle();

			try
			{
				std::ifstream file(path);

				if (file)
				{
					std::string b64_style;
					std::getline(file, b64_style);
					file.close();

					std::string decodeStyle = Base64::Decode(b64_style, b64_style.length());

					if (decodeStyle.size() != sizeof(ImGuiStyle))
					{
						this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet. Not ImGui 1.80 compatible.");
						return;
					}

					memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
				}
			}
			catch (...)
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet.");
			}

			break;
		}
		case EUIStyle::Code:
		{
			std::string decodeStyle = Base64::Decode(aValue, aValue.length());

			if (decodeStyle.size() != sizeof(ImGuiStyle))
			{
				this->Logger->Warning(CH_UICONTEXT, "Error applying stylesheet. Not ImGui 1.80 compatible.");
				return;
			}

			memcpy_s(style, sizeof(ImGuiStyle), &decodeStyle[0], decodeStyle.length());
			break;
		}
	}
}

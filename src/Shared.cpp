#include "Shared.h"

#include "Version.h"

HMODULE						D3D11Handle			= nullptr;
HMODULE						D3D11SystemHandle	= nullptr;

std::vector<std::string>	Parameters			= {};
std::vector<signed int>		RequestedAddons		= {};

ImGuiWindowFlags			WindowFlags_Default	=	ImGuiWindowFlags_AlwaysAutoResize |
													ImGuiWindowFlags_NoResize |
													ImGuiWindowFlags_NoCollapse;
ImGuiWindowFlags			WindowFlags_Overlay =	ImGuiWindowFlags_NoDecoration |
													ImGuiWindowFlags_AlwaysAutoResize |
													ImGuiWindowFlags_NoSavedSettings |
													ImGuiWindowFlags_NoFocusOnAppearing |
													ImGuiWindowFlags_NoNav |
													ImGuiWindowFlags_NoMove |
													ImGuiWindowFlags_NoInputs |
													ImGuiWindowFlags_NoBringToFrontOnFocus;
ImGuiWindowFlags			WindowFlags_Watermark = ImGuiWindowFlags_NoDecoration |
													ImGuiWindowFlags_AlwaysAutoResize |
													ImGuiWindowFlags_NoSavedSettings |
													ImGuiWindowFlags_NoFocusOnAppearing |
													ImGuiWindowFlags_NoNav |
													ImGuiWindowFlags_NoMove |
													ImGuiWindowFlags_NoInputs |
													ImGuiWindowFlags_NoBringToFrontOnFocus |
													ImGuiWindowFlags_NoBackground;

CApiClient*					RaidcoreAPI			= nullptr;
CApiClient*					GitHubAPI			= nullptr;

std::string					ChangelogText;
bool						IsUpdateAvailable	= false;

bool						IsGameLaunchSequence = true;

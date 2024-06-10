#include "Shared.h"

#include "Version.h"

DWORD						NexusModuleSize		= 0;
HMODULE						NexusHandle			= nullptr;
HMODULE						GameHandle			= nullptr;
HMODULE						D3D11Handle			= nullptr;
HMODULE						D3D11SystemHandle	= nullptr;

AddonVersion				Version				= { V_MAJOR, V_MINOR, V_BUILD, V_REVISION };
std::vector<std::string>	Parameters			= {};
std::vector<signed int>		RequestedAddons		= {};

CLocalization*				Language			= new CLocalization();
CUpdater*					UpdateService		= new CUpdater();
CTextureLoader*				TextureService		= new CTextureLoader();

ImFont*						MonospaceFont		= nullptr;
ImFont*						UserFont			= nullptr;
ImFont*						Font				= nullptr;
ImFont*						FontBig				= nullptr;
ImFont*						FontUI				= nullptr;

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
ImGuiWindowFlags			WindowFlags_Custom	=	ImGuiWindowFlags_AlwaysAutoResize |
													ImGuiWindowFlags_NoResize |
													ImGuiWindowFlags_NoCollapse |
													ImGuiWindowFlags_NoBackground |
													ImGuiWindowFlags_NoTitleBar;
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

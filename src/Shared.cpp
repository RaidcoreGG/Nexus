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

CLogHandler*				Logger				= new CLogHandler;
CLocalization*				Language			= new CLocalization();
CUpdater*					UpdateService		= new CUpdater();
CTextureLoader*				TextureService		= new CTextureLoader();
CDataLink*					DataLinkService		= new CDataLink();
CEventApi*					EventApi			= new CEventApi();
CRawInputApi*				RawInputApi			= new CRawInputApi();
CInputBindApi*				InputBindApi		= nullptr; //new CInputBindApi();
CGameBindsApi*				GameBindsApi		= nullptr; //new CGameBindsApi();
CUiContext*					UIContext			= nullptr;

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

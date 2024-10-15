#include "State.h"

#include <Windows.h>
#include <string>
#include <algorithm>
#include <shellapi.h>
#include <regex>
#include <type_traits>

#include "Consts.h"
#include "Shared.h"
#include "Index.h"

#include "Util/Strings.h"

namespace State
{
	ENexusState		Nexus						= ENexusState::NONE;
	EDxState		Directx						= EDxState::NONE;
	EEntryMethod	EntryMethod					= EEntryMethod::NONE;
	EMultiboxState	MultiboxState				= EMultiboxState::NONE;
	bool			IsChainloading				= false;

	bool			IsConsoleEnabled			= false;
	bool			IsVanilla					= false;

	std::string Initialize()
	{
		bool first = true;
		bool customMumble = false;
		std::string mumbleName;

		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		for (int i = 0; i < argc; i++)
		{
			std::wstring paramW = argv[i];
			std::string token = String::ToString(paramW);
			
			std::string cmp = token;

			if (first) { first = false; continue; } // skip location param

			cmp = String::ToLower(cmp);

			// token -> is the unmodified string as from the commandline
			// cmp -> same as token, except it's been normalised (aka written in lowercase)

			if (cmp == "-ggvanilla") { IsVanilla = true; }
			if (cmp == "-ggconsole") { IsConsoleEnabled = true; }

			if (cmp == "-mumble" && i + 1 <= argc)
			{
				std::wstring mumbleNameW = argv[i + 1];

				customMumble = true;
				mumbleName = String::ToString(mumbleNameW);

				token.append(" ");
				token.append(mumbleName);
				Parameters.push_back(token);
				i++; // manual increment to skip
			}
			else if (cmp == "-ggaddons" && i + 1 <= argc)
			{
				std::wstring addonIdsW = argv[i + 1];
				std::string addonIds = String::ToString(addonIdsW);

				std::vector<std::string> idList = String::Split(addonIds, ",");

				/* if -ggaddons param is config path, else it's id list*/
				if (idList.size() == 1 && String::Contains(idList[0], ".json"))
				{
					/* overwrite index path */
					Index::F_ADDONCONFIG = idList[0];
				}
				else
				{
					for (std::string addonId : idList)
					{
						try
						{
							signed int i = std::stoi(addonId);
							RequestedAddons.push_back(i);
						}
						catch (const std::invalid_argument& e)
						{
							Logger->Trace(CH_CORE, "Invalid argument (-ggaddons): %s (exc: %s)", addonId.c_str(), e.what());
						}
						catch (const std::out_of_range& e)
						{
							Logger->Trace(CH_CORE, "Out of range (-ggaddons): %s (exc: %s)", addonId.c_str(), e.what());
						}
					}
				}

				token.append(" ");
				token.append(addonIds);
				Parameters.push_back(token);
				i++; // manual increment to skip
			}
			else
			{
				Parameters.push_back(token);
			}
		}

		if (!customMumble)
		{
			mumbleName = "MumbleLink";
		}

		return mumbleName;
	}
}

EMultiboxState operator|(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>(std::underlying_type_t<EMultiboxState>(lhs) | std::underlying_type_t<EMultiboxState>(rhs));
}
EMultiboxState operator&(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>(std::underlying_type_t<EMultiboxState>(lhs) & std::underlying_type_t<EMultiboxState>(rhs));
}
EMultiboxState operator|=(EMultiboxState& lhs, EMultiboxState rhs)
{
	return lhs = lhs | rhs;
}
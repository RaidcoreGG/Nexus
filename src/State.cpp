#include "State.h"

namespace State
{
	ENexusState		Nexus						= ENexusState::NONE;
	EDxState		Directx						= EDxState::NONE;
	EEntryMethod	EntryMethod					= EEntryMethod::NONE;
	EMultiboxState	MultiboxState				= EMultiboxState::NONE;
	bool			IsChainloading				= false;
	bool			IsImGuiInitialized			= false;

	bool			IsDeveloperMode				= false;
	bool			IsConsoleEnabled			= false;
	bool			IsVanilla					= false;
	bool			IsMumbleDisabled			= false;

	void Initialize()
	{
		bool first = true;
		bool customMumble = false;

		std::string cmdline = GetCommandLineA();
		cmdline.append(" -;"); // append another space, so the last substring can be ignored
		std::string delimiter = " -";

		size_t pos = 0;
		std::string token;
		while ((pos = cmdline.find(delimiter)) != std::string::npos)
		{
			token = cmdline.substr(0, pos);
			std::string cmp = token;
			cmdline.erase(0, pos + delimiter.length());

			if (first) { first = false; continue; } // skip location param

			Parameters.push_back(token);
			std::transform(cmp.begin(), cmp.end(), cmp.begin(), ::tolower);

			//Log("dbg", "token: \"%s\" @ %d", token.c_str(), pos);

			// token -> is the unmodified string as from the commandline
			// cmp -> same as token, except it's been normalised (aka written in lowercase)

			if (cmp == "ggdev") { IsDeveloperMode = true; }
			if (cmp == "ggconsole") { IsConsoleEnabled = true; }
			if (cmp == "ggvanilla") { IsVanilla = true; }
			if (cmp == "sharearchive") { MultiboxState = MultiboxState | EMultiboxState::ARCHIVE_SHARED; }
			if (cmp == "multi") { MultiboxState = MultiboxState | EMultiboxState::LOCAL_SHARED; }

			size_t subpos = 0;
			std::string subtoken;
			if ((subpos = cmp.find("mumble ")) != std::string::npos)
			{
				subtoken = token.substr(7, token.length() - subpos);
				if (std::regex_match(subtoken, std::regex("\"(.*?)\"")))
				{
					subtoken = subtoken.substr(1, subtoken.length() - 2);
				}
				//Log("dbg", "subtoken: \"%s\" @ %d", subtoken.c_str(), subpos);

				customMumble = true;
				MumbleLink = (LinkedMem*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(LinkedMem), subtoken.c_str());
			}
		}

		if (!customMumble)
		{
			MumbleLink = (LinkedMem*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(LinkedMem), "MumbleLink");
		}

		// TODO:
		// close "AN-Mutex-Window-Guild Wars 2"
	}
}

EMultiboxState operator|(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
EMultiboxState operator&(EMultiboxState lhs, EMultiboxState rhs)
{
	return static_cast<EMultiboxState>( static_cast<int>(lhs) & static_cast<int>(rhs) );
}
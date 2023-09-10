#include "State.h"

namespace State
{
	ENexusState		AddonHost					= ENexusState::NONE;
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

			if (token == "ggdev") { IsDeveloperMode = true; }
			if (token == "ggconsole") { IsConsoleEnabled = true; }
			if (token == "ggvanilla") { IsVanilla = true; }

			size_t subpos = 0;
			std::string subtoken;
			if ((subpos = cmp.find("mumble ")) != std::string::npos)
			{
				subtoken = token.substr(7, token.length() - subpos);
				//Log("dbg", "subtoken: \"%s\" @ %d", subtoken.c_str(), subpos);

				customMumble = true;
				MumbleLink = (LinkedMem*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(LinkedMem), subtoken);
			}
		}

		if (!customMumble)
		{
			MumbleLink = (LinkedMem*)DataLink::ShareResource(DL_MUMBLE_LINK, sizeof(LinkedMem), "MumbleLink");
		}
	}
}
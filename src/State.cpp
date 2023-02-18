#include "State.h"

namespace State
{
	ggState			AddonHost					= ggState::NONE;
	EDxState		Directx						= EDxState::NONE;
	EEntryMethod	EntryMethod					= EEntryMethod::NONE;
	bool			IsChainloading				= false;
	bool			IsImGuiInitialized			= false;

	bool			IsDeveloperMode				= false;
	bool			IsConsoleEnabled = false;
	bool			IsVanilla					= false;
	bool			IsMumbleDisabled			= false;

	void Initialize()
	{
		/* arg list */
		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		bool customMumble = false;

		/* skip first, that's the file path */
		for (int i = 1; i < argc; ++i)
		{
			std::wstring str;
			std::wstring cmp = argv[i];
			std::transform(cmp.begin(), cmp.end(), cmp.begin(), ::tolower);

			str.append(argv[i]);

			/* peek at the next argument, if it starts with - */
			if (i + 1 < argc && argv[i + 1][0] != L'-')
			{
				/* next argument belongs to this one */
				if (wcscmp(cmp.c_str(), L"-mumble") == 0)
				{
					bool customMumble = true;
					MumbleLink = Mumble::Initialize(argv[i + 1]);
				}

				str.append(L" ");
				str.append(argv[i + 1]);
				i++;
			}
			else
			{
				/* single argument */
#ifdef _DEBUG
				State::IsDeveloperMode = true;
#else
				State::IsDeveloperMode = wcscmp(cmp.c_str(), L"-ggdev") == 0;
				State::IsConsoleEnabled = wcscmp(cmp.c_str(), L"-ggconsole") == 0;
#endif
				State::IsVanilla = wcscmp(cmp.c_str(), L"-ggvanilla") == 0;
			}

			Parameters.push_back(str);
		}
		if (!customMumble) { MumbleLink = Mumble::Initialize(L"MumbleLink"); }
	}
}
///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmdLine.cpp
/// Description  :  Contains functions for the commandline.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CmdLine.h"

#include <Windows.h>
#include <shellapi.h>

#include "Util/Strings.h"

namespace CmdLine
{
	static std::vector<std::string> s_Arguments;

	bool HasArgument(const std::string& aArgument)
	{
		if (s_Arguments.size() == 0)
		{
			Parse();
		}

		std::string arg = String::ToLower(aArgument);

		for (const std::string& str : s_Arguments)
		{
			std::string cmp = String::ToLower(str);

			if (arg == cmp) { return true; }
		}

		return false;
	}

	std::string GetArgumentValue(const std::string& aArgument)
	{
		if (s_Arguments.size() == 0)
		{
			Parse();
		}

		std::string arg = String::ToLower(aArgument);

		for (size_t i = 0; i < s_Arguments.size(); i++)
		{
			std::string cmp = String::ToLower(s_Arguments[i]);

			if (arg == cmp)
			{
				if (i < s_Arguments.size() - 1)
				{
					const std::string& argVal = s_Arguments[i + 1];

					return argVal;
				}

				break;
			}
		}

		return "";
	}

	std::vector<std::string> Parse()
	{
		s_Arguments.clear();

		std::vector<std::string> args; // extra vector to return value arguments merged

		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		for (int i = 0; i < argc; i++)
		{
			std::wstring paramW = argv[i];
			std::string token = String::ToString(paramW);

			s_Arguments.push_back(token);

			if (String::StartsWith(token, "-"))
			{
				args.push_back(token);
			}
			else
			{
				if (args.size() >= 1)
				{
					args[args.size() - 1].append(" " + token);
				}
				else
				{
					args.push_back(token);
				}
			}
		}

		return args;
	}
}

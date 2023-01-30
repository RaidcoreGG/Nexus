#include <cstdarg>
#include <string>

#include "KeybindHandler.h"

#include "../core.h"
#include "../Paths.h"
#include "../Shared.h"
#include "../State.h"

using json = nlohmann::json;

namespace KeybindHandler
{
	std::mutex KeybindRegistryMutex;
	std::map<const wchar_t*, Keybind> KeybindRegistry;
	std::map<const wchar_t*, KEYBINDS_PROCESS> KeybindHandlerRegistry;

	std::mutex HeldKeysMutex;
	std::vector<WPARAM> HeldKeys;

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Keybind kb{};
		kb.Alt		= GetKeyState(VK_MENU)		& 0x8000;
		kb.Ctrl		= GetKeyState(VK_CONTROL)	& 0x8000;
		kb.Shift	= GetKeyState(VK_SHIFT)		& 0x8000;

		switch (uMsg)
		{
		case WM_SYSKEYDOWN: case WM_KEYDOWN:
			if (wParam > 255) break;
			kb.Key = wParam;
			
			HeldKeysMutex.lock();

			if (std::find(HeldKeys.begin(), HeldKeys.end(), wParam) != HeldKeys.end())
			{
				HeldKeysMutex.unlock();
				return false;
			}
			HeldKeys.push_back(wParam);

			HeldKeysMutex.unlock();
			
			for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
			{
				Keybind stored = it->second;

				if (kb == stored)
				{
					InvokeKeybind(it->first);
					return true;
				}
			}
		case WM_SYSKEYUP: case WM_KEYUP:
			HeldKeysMutex.lock();

			HeldKeys.erase(std::remove(HeldKeys.begin(), HeldKeys.end(), wParam), HeldKeys.end());

			HeldKeysMutex.unlock();
			break;
		}

		return false;
	}

	void LoadKeybinds()
	{
		if (!std::filesystem::exists(Path::F_KEYBINDS)) { return; }

		KeybindRegistryMutex.lock();

		std::wifstream file(Path::F_KEYBINDS);

		//json keybinds = json::parse(file);

		std::wstring line;
		while (std::getline(file, line))
		{
			std::transform(line.begin(), line.end(), line.begin(), ::toupper);
			std::wstring delimiter = L"=";

			size_t pos = 0;
			std::wstring token;
			while ((pos = line.find(delimiter)) != std::wstring::npos)
			{
				token = line.substr(0, pos);
				line.erase(0, pos + delimiter.length());
			}

			wchar_t* id = new wchar_t[64];
			wchar_t* kb = new wchar_t[64];
			wmemcpy(id, token.c_str(), token.size() + 1);
			wmemcpy(kb, line.c_str(), line.size() + 1);

			KeybindRegistry[id] = KBFromString(kb);
			LogDebug(L"%s | %s", id, kb);
		}

		/*while (file)
		{
			wchar_t* id = new wchar_t[64];
			wchar_t* kb = new wchar_t[64];

			file.read(id, 64);
			file.read(kb, 64);
		}*/

		file.close();

		/*for (json bind : keybinds)
		{
			std::string idStr = bind["Identifier"].get<std::string>();
			std::string kbStr = bind["Binding"].get<std::string>();

			std::wstring idStrW = StrToWStr(idStr);
			std::wstring kbStrW = StrToWStr(kbStr);

			const wchar_t* idW = idStrW.c_str();
			const wchar_t* kbW = kbStrW.c_str();

			int idSz = wcslen(idW) + 2;
			int kbSz = wcslen(kbW) + 2;

			wchar_t* id = new wchar_t[idSz];
			wchar_t* kb = new wchar_t[kbSz];

			wmemcpy(id, idW, idSz);
			wmemcpy(kb, kbW, kbSz);

			KeybindRegistry[id] = KBFromString(kb);
			LogDebug(L"%s | %s", id, kb);
		}*/

		KeybindRegistryMutex.unlock();
	}

	void SaveKeybinds()
	{
		KeybindRegistryMutex.lock();

		//json keybinds = json::array();

		std::wofstream file(Path::F_KEYBINDS);

		for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
		{
			std::wstring id = it->first;
			std::wstring kb = it->second.ToString();

			/*json binding =
			{
				{"Identifier",	WStrToStr(id)},
				{"Binding",		WStrToStr(kb)},
			};

			keybinds.push_back(binding);*/

			file.write(id.c_str(), id.size());
			file.write(L"=", 1);
			file.write(kb.c_str(), kb.size());
			file.write(L"\n", 1);
		}


		//file << keybinds.dump(1, '\t').c_str() << std::endl;
		file.close();

		KeybindRegistryMutex.unlock();
	}

	void RegisterKeybind(const wchar_t* aIdentifier, KEYBINDS_PROCESS aKeybindHandler, const wchar_t* aKeybind)
	{
		Keybind requestedBind = KBFromString(aKeybind);

		KeybindRegistryMutex.lock();

		/* check if another identifier, already uses the keybind */
		for (auto& [identifier, keybind] : KeybindRegistry)
		{
			LogDebug(L"%s | %s", identifier, keybind.ToString().c_str());

			if (keybind == requestedBind)
			{
				Log("same combo found");

				if (wcscmp(identifier, aIdentifier) != 0)
				{
					/* another identifier uses the same combination */
					requestedBind = {};
					Log("diff identifier");
				}

				break;
			}
		}

		/* check if this keybind is not already set */
		if (KeybindRegistry.find(aIdentifier) == KeybindRegistry.end())
		{
			Log(aIdentifier);
			KeybindRegistry[aIdentifier] = requestedBind;
			Log("identifier not mapped");
		}
		else
		{
			Log(aIdentifier);
			Log("identifier already mapped");
		}

		KeybindHandlerRegistry[aIdentifier] = aKeybindHandler;

		KeybindRegistryMutex.unlock();

		SaveKeybinds();
	}

	void UnregisterKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		KeybindRegistry.erase(aIdentifier);
		KeybindHandlerRegistry.erase(aIdentifier);

		KeybindRegistryMutex.unlock();
	}

	void InvokeKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		if (KeybindHandlerRegistry[aIdentifier])
		{
			KeybindHandlerRegistry[aIdentifier](aIdentifier);
		}

		KeybindRegistryMutex.unlock();
	}
}
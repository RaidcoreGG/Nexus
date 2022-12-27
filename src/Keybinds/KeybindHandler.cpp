#include "KeybindHandler.h"

bool operator==(const Keybind& lhs, const Keybind& rhs)
{
	return	lhs.Key		== rhs.Key &&
			lhs.Alt		== rhs.Alt &&
			lhs.Ctrl	== rhs.Ctrl &&
			lhs.Shift	== rhs.Shift;
}

namespace KeybindHandler
{
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
			
			for (std::map<const wchar_t*, Keybind>::iterator it = KeybindRegistry.begin(); it != KeybindRegistry.end(); ++it)
			{
				Keybind stored = it->second;

				if (kb == stored)
				{
					InvokeKeybind(it->first);
					return true;
				}
			}
		}

		return false;
	}

	void LoadKeybinds()
	{
		
	}

	void SaveKeybinds()
	{

	}

	void RegisterKeybind(const wchar_t* aIdentifier, KeybindHandlerSig aKeybindHandler, Keybind aKeybind)
	{
		KeybindRegistryMutex.lock();

		KeybindHandlerRegistry[aIdentifier] = aKeybindHandler;
		KeybindRegistry[aIdentifier] = aKeybind;

		KeybindRegistryMutex.unlock();
	}

	void InvokeKeybind(const wchar_t* aIdentifier)
	{
		KeybindRegistryMutex.lock();

		KeybindHandlerRegistry[aIdentifier](aIdentifier);

		KeybindRegistryMutex.unlock();
	}
}
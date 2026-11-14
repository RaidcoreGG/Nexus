///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MainWindow.h
/// Description  :  Contains the logic for the main Nexus UI window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <vector>

#include "Core/Preferences/PrefSetting.h"
#include "Host/GW2/Addons/AddNotify.h"
#include "UI/Controls/CtlSubWindow.h"
#include "UI/Controls/CtlWindow.h"
#include "UI/Textures/TxTexture.h"

///----------------------------------------------------------------------------------------------------
/// CMainWindow Class
///----------------------------------------------------------------------------------------------------
class CMainWindow : public virtual IWindow, public IAddonStateNotifyHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnInputBind:
	/// 	Invokes an input bind.
	///----------------------------------------------------------------------------------------------------
	static void OnInputBind(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// OnAddonStateNotify:
	/// 	Brings up the addons window when a volatile addon was disabled.
	///----------------------------------------------------------------------------------------------------
	void OnAddonStateNotify(EAddonStateNotify aState, uint32_t aSignature) override;

	CMainWindow();

	~CMainWindow();

	void AddWindow(ISubWindow* aWindow);
	void RemoveWindow(ISubWindow* aWindow);

	void Render() override;

	void Invalidate() override;

	void Activate(const std::string& aWindowName = "");

	private:
	std::mutex               Mutex          {};
	std::vector<ISubWindow*> Windows        {};
	ISubWindow*              ActiveContent  {};
	bool                     IsHandleHeld   {};

	private:
	Texture_t*               Tex_RaidcoreTag{};
	Texture_t*               Tex_CloseIcon  {};

	public:
	CSetting<bool>           ShowAddonsAfterGameUpdate{ "ShowAddonsWindowAfterDisableUntilUpdate", false };
};

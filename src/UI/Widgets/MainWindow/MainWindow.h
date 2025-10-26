///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MainWindow.h
/// Description  :  Contains the logic for the main Nexus UI window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mutex>
#include <vector>

#include "UI/Controls/CtlWindow.h"
#include "UI/Controls/CtlSubWindow.h"
#include "Engine/Textures/TxTexture.h"

///----------------------------------------------------------------------------------------------------
/// CMainWindow Class
///----------------------------------------------------------------------------------------------------
class CMainWindow : public virtual IWindow
{
	public:
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
};

#endif

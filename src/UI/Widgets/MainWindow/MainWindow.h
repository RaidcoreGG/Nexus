///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MainWindow.h
/// Description  :  Contains the logic for the main Nexus UI window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "UI/Controls/CtlWindow.h"

constexpr const char* KB_MENU = "KB_MENU";

///----------------------------------------------------------------------------------------------------
/// CMainWindow Class
///----------------------------------------------------------------------------------------------------
class CMainWindow
{
	void AddWindow(IWindow* aWindow);
	void RemoveWindow();
};

#endif

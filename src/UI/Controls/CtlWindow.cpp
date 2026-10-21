///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlWindow.cpp
/// Description  :  Contains the functionality for a window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CtlWindow.h"

const std::string& IWindow::GetName()
{
	return this->Name;
}

const std::string& IWindow::GetDisplayName()
{
	return this->DisplayName;
}

void IWindow::Invalidate()
{
	this->IsInvalid = true;
}

bool* IWindow::GetVisibleStatePtr()
{
	return &this->IsVisible;
}

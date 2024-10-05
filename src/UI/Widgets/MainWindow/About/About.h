///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  About.h
/// Description  :  Contains the content of the about window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_ABOUT_H
#define MAINWINDOW_ABOUT_H

#include "UI/Controls/CtlSubWindow.h"
#include "Services/Textures/Texture.h"

class CAboutBox : public ISubWindow
{
	public:
	CAboutBox();
	void RenderContent() override;

	private:
	Texture* Tex_BannerDiscord = nullptr;
	Texture* Tex_BannerPatreon = nullptr;
};

#endif

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
#include "Engine/Textures/TxTexture.h"

class CAboutBox : public ISubWindow
{
	public:
	CAboutBox();
	void RenderContent() override;

	private:
	Texture_t* Tex_BannerDiscord = nullptr;
	Texture_t* Tex_BannerPatreon = nullptr;
};

#endif

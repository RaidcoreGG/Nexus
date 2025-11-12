///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  About.h
/// Description  :  Contains the content of the about window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "UI/Controls/CtlSubWindow.h"
#include "UI/Textures/TxTexture.h"

class CAboutBox : public ISubWindow
{
	public:
	CAboutBox();
	void RenderContent() override;

	private:
	Texture_t* Tex_BannerDiscord = nullptr;
	Texture_t* Tex_BannerPatreon = nullptr;
};

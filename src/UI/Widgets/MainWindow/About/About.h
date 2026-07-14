///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  About.h
/// Description  :  Contains the content of the about window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "UI/Controls/CtlSubWindow.h"
#include "Graphics/Textures/TxTexture.h"

using namespace Raidcore::Nexus;

class CAboutBox : public ISubWindow
{
	public:
	CAboutBox();
	void RenderContent() override;

	private:
	Graphics::Texture_t* Tex_BannerDiscord = nullptr;
	Graphics::Texture_t* Tex_BannerPatreon = nullptr;
};

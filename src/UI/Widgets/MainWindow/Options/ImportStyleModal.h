///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ImportStyleModal.h
/// Description  :  Modal for style importing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IMPORTSTYLEMODAL_H
#define IMPORTSTYLEMODAL_H

#include "UI/Controls/CtlModal.h"

///----------------------------------------------------------------------------------------------------
/// CImportStyleModal Class
///----------------------------------------------------------------------------------------------------
class CImportStyleModal : public virtual IModal
{
	public:
	CImportStyleModal();

	void RenderContent() override;

	void OnClosing() override;

	private:
	char DataBuffer[4096]; /* Very big buffer, to store the entire Base64 code. */
};

#endif

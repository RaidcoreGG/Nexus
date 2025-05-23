///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LicenseAgreementModal.h
/// Description  :  Modal for license agreement.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LICENSEAGREEMENTMODAL_H
#define LICENSEAGREEMENTMODAL_H

#include "UI/Controls/CtlModal.h"

///----------------------------------------------------------------------------------------------------
/// CLicenseAgreementModal Class
///----------------------------------------------------------------------------------------------------
class CLicenseAgreementModal : public virtual IModal
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLicenseAgreementModal();

	///----------------------------------------------------------------------------------------------------
	/// RenderContent:
	/// 	Render function of modal contents.
	///----------------------------------------------------------------------------------------------------
	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// OnClosing:
	/// 	Override close to process result.
	///----------------------------------------------------------------------------------------------------
	void OnClosing() override;
};

#endif

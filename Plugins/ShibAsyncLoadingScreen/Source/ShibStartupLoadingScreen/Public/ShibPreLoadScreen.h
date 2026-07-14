// Copyright Shiba Inu Games LLC.

#pragma once

#include "PreLoadScreenBase.h"

class SWidget;

class SHIBSTARTUPLOADINGSCREEN_API FShibPreLoadScreen : public FPreLoadScreenBase
{
public:
	
	/*** IPreLoadScreen Implementation ***/
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override;
	virtual TSharedPtr<SWidget> GetWidget()  override { return EngineLoadingWidget; };

private:

	TSharedPtr<SWidget> EngineLoadingWidget;
};

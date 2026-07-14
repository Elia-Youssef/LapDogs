// Copyright Shiba Inu Games LLC.

#include "ShibPreLoadScreen.h"
#include "Misc/App.h"
#include "ShibPreLoadingScreenWidget.h"

void FShibPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SShibPreLoadingScreenWidget);
	}
}

EPreLoadScreenTypes FShibPreLoadScreen::GetPreLoadScreenType() const
{
	return EPreLoadScreenTypes::EngineLoadingScreen;
}
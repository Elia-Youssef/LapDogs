// Copyright Shiba Inu Games LLC.

#include "ShibStartupLoadingScreen.h"
#include "PreLoadScreenManager.h"
#include "ShibPreLoadScreen.h"

void FShibStartupLoadingScreenModule::StartupModule()
{
	// No need to load these assets on dedicated servers.
	// Still want to load them in commandlets so cook catches them
	if (!IsRunningDedicatedServer())
	{
		PreLoadingScreen = MakeShared<FShibPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FShibStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

void FShibStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	//Once the PreLoadScreenManager is cleaning up, we can get rid of all our resources too
	PreLoadingScreen.Reset();
	ShutdownModule();
}

void FShibStartupLoadingScreenModule::ShutdownModule()
{
	
}

bool FShibStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

IMPLEMENT_MODULE(FShibStartupLoadingScreenModule, ShibStartupLoadingScreen)
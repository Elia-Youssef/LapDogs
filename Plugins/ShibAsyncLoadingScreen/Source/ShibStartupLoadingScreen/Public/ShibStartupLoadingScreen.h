// Copyright Shiba Inu Games LLC.

#pragma once

#include "ShibPreLoadScreen.h"
#include "Modules/ModuleManager.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"

class FShibStartupLoadingScreenModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    bool IsGameModule() const override;

private:
    void OnPreLoadScreenManagerCleanUp();

    TSharedPtr<FShibPreLoadScreen> PreLoadingScreen;
};
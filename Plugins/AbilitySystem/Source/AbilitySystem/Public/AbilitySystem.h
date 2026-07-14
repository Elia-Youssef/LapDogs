// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAbilitySystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

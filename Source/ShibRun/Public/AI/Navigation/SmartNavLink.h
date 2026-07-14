// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "SmartNavLink.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API ASmartNavLink : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	ASmartNavLink();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void SmartLinkReached(AActor* MovingActor, const FVector& DestinationPoint);
	
};

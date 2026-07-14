// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibUiBasePage.h"
#include "ShibBaseNavigation.h"
#include "Player/ShibBasePlayerState.h"
#include "ShibBasePage.generated.h"

class AShibBaseGameState;
class AShiBaseGameState;
class AShibBaseController;
class UShibGameInstance;
class UShibBaseNavigation;

/**
 * 
 */
UCLASS()
class SHIBRUN_API UShibBasePage : public UShibUiBasePage
{
	GENERATED_BODY()
	
public:
	/**
	 * Reference to parent class `UShibBaseNavigation`
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibUiBaseNavigation> BaseNavigation;
	
	// ---- Getters ----
	
	UPROPERTY()
	TObjectPtr<UShibGameInstance> ShibGI;
	
	UFUNCTION(BlueprintPure)
	UShibGameInstance* GetShibGI();
	
	UPROPERTY()
	TObjectPtr<AShibBaseController> ShibBaseCtrl;
	
	UFUNCTION(BlueprintPure)
	AShibBaseController* GetShibBaseController();
	
	UPROPERTY()
	TObjectPtr<AShibBasePlayerState> ShibBasePS;
	
	UFUNCTION(BlueprintPure)
	AShibBasePlayerState* GetShibBasePS();
	
	UPROPERTY()
	TObjectPtr<AShibBaseGameState> ShibBaseGS;
	
	UFUNCTION(BlueprintPure)
	AShibBaseGameState* GetShibBaseGS();

protected:
	virtual void NativeConstruct() override;
};

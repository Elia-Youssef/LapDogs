// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibUiBaseNavigation.h"
#include "Blueprint/UserWidget.h"
#include "ShibUiBasePage.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPageChangeDelegate, ENavigationDirection, NavigationDirection, FPageDetails, PageDetails);

class UShibUiBaseNavigation;

UCLASS()
class SHIBUINAVIGATION_API UShibUiBasePage : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/**
	 * Z order of the page inside it's canvas container
	 */
	UPROPERTY(EditDefaultsOnly, Category=ShibPageSettings)
	int32 PageZOrder = 0;
	
	/**
	 * Time to wait before removing page from parent.
	 */
	UPROPERTY(EditDefaultsOnly, Category=ShibPageSettings, meta = (ClampMin = "0", ForceUnits=s))
	float PageLifespan = 0.f;

	/**
	 * Expected time for the `DestroyPage` event to finish.
	 * Seconds to wait before constructing next/previous page.
	 */
	UPROPERTY(EditDefaultsOnly, Category=ShibPageSettings, meta = (ClampMin = "0", ForceUnits=s))
	float TimeNeededToDestroy = 0.f;

	/**
	 * Reference to parent class `UShibUiBaseNavigation`
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibUiBaseNavigation> ShibNavigation;

	UPROPERTY()
	FOnPageChangeDelegate OnPageChangeDelegate;

	/**
	 * Gets called every time a page changes while this page is in the history navigation
	 * @param NavigationDirection Forward/Backward
	 * @param PageDetails New page details
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnPageChange(ENavigationDirection NavigationDirection, FPageDetails PageDetails);
	
	virtual void Internal_ConstructPage(ENavigationDirection NavigationDirection);
	virtual void Internal_DestroyPage(ENavigationDirection NavigationDirection, bool bRemoveFromParent = false);

	/**
	 * This function is called when this page is opened.
	 * Put widget construction functions in the `NativeConstruct` event.
	 * Put UI construction functions here (ex: animations...).
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ConstructPage(ENavigationDirection NavigationDirection);
	
	/**
	 * This function is called when this page is closed.
	 * Put widget destruction functions in the `NativeDestruct` event.
	 * Put UI destruction functions here (ex: animations...).
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void DestroyPage(ENavigationDirection NavigationDirection);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// Override the native tick function
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	FTimerHandle LifespanHandle;
	FTimerDelegate LifespanDelegate;

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "ShibUiBasePage.h"
#include "Kismet/GameplayStatics.h"

void UShibUiBasePage::NativeConstruct()
{
	Super::NativeConstruct();

	OnPageChangeDelegate.AddUniqueDynamic(this, &ThisClass::OnPageChange);
}

void UShibUiBasePage::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(LifespanHandle);
	LifespanDelegate.Unbind();
	OnPageChangeDelegate.Clear();
}

void UShibUiBasePage::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	// Check if the Game is not Paused
	if (!UGameplayStatics::IsGamePaused(GetWorld()))
	{
		// Call the parent implementation
		Super::NativeTick(MyGeometry, InDeltaTime);
	}
}

#if WITH_EDITOR
void UShibUiBasePage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	PageLifespan = TimeNeededToDestroy>PageLifespan ? TimeNeededToDestroy : PageLifespan;
}
#endif

void UShibUiBasePage::Internal_ConstructPage(ENavigationDirection NavigationDirection)
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ConstructPage(NavigationDirection);
}

void UShibUiBasePage::Internal_DestroyPage(ENavigationDirection NavigationDirection, bool bRemoveFromParent)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	DestroyPage(NavigationDirection);
	
	if (bRemoveFromParent)
	{
		if (PageLifespan>0)
		{
			LifespanDelegate.BindLambda([this]()
			{
				RemoveFromParent();
			});
			GetWorld()->GetTimerManager().SetTimer(LifespanHandle, LifespanDelegate, PageLifespan, false);
		}
		else
		{
			RemoveFromParent();
		}
	}
}

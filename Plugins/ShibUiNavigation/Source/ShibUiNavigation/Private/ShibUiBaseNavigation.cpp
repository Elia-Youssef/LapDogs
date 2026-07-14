// Fill out your copyright notice in the Description page of Project Settings.


#include "ShibUiBaseNavigation.h"
#include "ShibUiBasePage.h"
#include "ShibNavigationSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

void UShibUiBaseNavigation::NativeConstruct()
{
	/* // Removed for now until we find a better way of managing widget reference and destruction
	if (NavigationHistory.IsEmpty())
	{
		NavigationHistory = GetHistoryFromGi();
	}
	*/
	
	Super::NativeConstruct();
}

void UShibUiBaseNavigation::NativeDestruct()
{
	if (IsValid(CurrentPage.PageRef))
	{
		CurrentPage.PageRef->RemoveFromParent();
		CurrentPage = FPageDetails();
	}
	
	Super::NativeDestruct();
}

bool UShibUiBaseNavigation::IsPageInHistory(const FPageDetails PageToFind)
{
	if (NavigationHistory.IsEmpty()) return false;
	
	for (auto Page : NavigationHistory)
	{
		if (PageToFind == Page)
			return true;
	}

	return false;
}

bool UShibUiBaseNavigation::FindPageInHistory(FName PageIdToFind, FPageDetails& PageFound)
{
	if (NavigationHistory.IsEmpty()) return false;
	
	for (auto Page : NavigationHistory)
	{
		if (Page.PageId == PageIdToFind)
		{
			PageFound = Page;
			return true;	
		}
	}
	
	return false;
}

void UShibUiBaseNavigation::GoBack()
{
	if (NavigationHistory.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("[GoBack] Navigation history empty"))
		return;
	}
	
	FName PageToGo = CurrentPage.PreviousPageId;
	
	if (IsPageInHistory(CurrentPage))
	{
		PopHistory(PageToGo);
		PrevPageDestructionTime = CurrentPage.PageRef->TimeNeededToDestroy;
		CurrentPage.PageRef = nullptr;
		GoToPage(PageToGo, true, ENavigationDirection::NavBackward);
		return;
	}
	
	if (IsValid(CurrentPage.PageRef))
		CurrentPage.PageRef->Internal_DestroyPage(NavBackward, true);
	
	PrevPageDestructionTime = CurrentPage.PageRef->TimeNeededToDestroy;
	CurrentPage.PageRef = nullptr;

	GoToPage(PageToGo, true, ENavigationDirection::NavBackward);
}

void UShibUiBaseNavigation::GoToLastPageInHistory(const FName FallbackPageId)
{
	if (NavigationHistory.IsEmpty())
	{
		GoToPage(FallbackPageId);
		return;
	}
	
	GoToPage(NavigationHistory.Last().PageId);
}

void UShibUiBaseNavigation::GoToPage_Implementation(const FName PageId, const bool bKeepInHistory, ENavigationDirection NavigationDirection)
{
	if (!Pages.Contains(PageId) || !IsValid(Pages[PageId]))
	{
		UE_LOG(LogTemp, Display, TEXT("[GoToPage] Invalid page"))
		return;
	}

	// If the page is already open, we do nothing
	if (CurrentPage.PageId == PageId && CurrentPage.PageRef) return;
	
	// If our current page is still valid, we destroy it
	if (CurrentPage.PageRef)
	{
		PrevPageDestructionTime = CurrentPage.PageRef->TimeNeededToDestroy;

		CurrentPage.PageRef->Internal_DestroyPage(NavigationDirection, !IsPageInHistory(CurrentPage) /*false when current page is inside the history, soft destroy*/);
		
		CurrentPage.PageRef = nullptr;
	}

	// We check if the page we want to go to is already inside the history
	FPageDetails PageToFind;
	if (FindPageInHistory(PageId, PageToFind))
	{
		CurrentPage = PageToFind;
		AddCurrentPageToViewport(NavigationDirection);
		return;
	}

	// At this point, we don't have any current page and we need to create the new one
	CurrentPage.PreviousPageId = CurrentPage.PageId;
	CurrentPage.PageId = PageId;
	CurrentPage.PageRef = CreateWidget<UShibUiBasePage>(GetOwningPlayer(), Pages[PageId]);

	if (bKeepInHistory) AddToHistory(CurrentPage);
	
	AddCurrentPageToViewport(NavigationDirection);
}

void UShibUiBaseNavigation::AddCurrentPageToViewport(const ENavigationDirection NavigationDirection)
{
	CurrentPage.PageRef->ShibNavigation = this;

	if (PrevPageDestructionTime>0.f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UShibUiBaseNavigation::InitCurrentPage, NavigationDirection);
		GetWorld()->GetTimerManager().SetTimer(InitPageDurationHandle, Delegate, PrevPageDestructionTime, false);
	}
	else
	{
		InitCurrentPage(NavigationDirection);
	}
}

void UShibUiBaseNavigation::InitCurrentPage(ENavigationDirection NavigationDirection)
{
	if(!CurrentPage.PageRef) return;
	
	if (!CurrentPage.PageRef->IsInViewport())
	{
		auto NewPanel = CanvasContainer->AddChildToCanvas(CurrentPage.PageRef);
		NewPanel->SetAnchors(FAnchors{0.f, 0.f, 1.f, 1.f});
		NewPanel->SetOffsets(FMargin{0.f,0.f,0.f,0.f});
		NewPanel->SetZOrder( CurrentPage.PageRef->PageZOrder);
	}

	CurrentPage.PageRef->Internal_ConstructPage(NavigationDirection);
	
	// notify page change delegate
	for (FPageDetails& Page : NavigationHistory)
	{
		if (IsValid(Page.PageRef) && Page.PageRef != CurrentPage.PageRef)
			Page.PageRef->OnPageChangeDelegate.Broadcast(NavigationDirection, CurrentPage);
	}
}

void UShibUiBaseNavigation::PopHistory(const FName PopToPageId)
{
	FPageDetails PopToPage;
	PopToPage.PageId = PopToPageId;
	
	if (IsPageInHistory(PopToPage))
	{
		for (int32 i=NavigationHistory.Num()-1; i>=0; i--)
		{
			if (NavigationHistory[i].PageId == PopToPage.PageId) break;

			if (IsValid(NavigationHistory[i].PageRef))
				NavigationHistory[i].PageRef->Internal_DestroyPage(NavBackward, true);
			
			NavigationHistory.RemoveAt(i);
		}
	}
	
	// Removed for now until we find a better way of managing widget reference and destruction
	//SetGiHistory(NavigationHistory);
}

void UShibUiBaseNavigation::AddToHistory(const FPageDetails& Page)
{
	NavigationHistory.Add(Page);
	// Removed for now until we find a better way of managing widget reference and destruction
	//SetGiHistory(NavigationHistory);
}

TArray<FPageDetails> UShibUiBaseNavigation::GetHistoryFromGi()
{
	if (!GetShibNavigationSubsystem()) return TArray<FPageDetails>();
	
	for (FNavigationHistory& GiHistory : ShibNavigationSubsystem->NavigationHistory)
	{
		if (GiHistory.UniqueId == NavigationUniqueId)
		{
			return GiHistory.NavigationHistory;
		}
	}
	
	return TArray<FPageDetails>();
}

void UShibUiBaseNavigation::SetGiHistory(const TArray<FPageDetails>& History)
{
	if (!GetShibNavigationSubsystem()) return;
	
	for (FNavigationHistory& GiHistory : GetShibNavigationSubsystem()->NavigationHistory)
	{
		if (GiHistory.UniqueId == NavigationUniqueId)
		{
			GiHistory.NavigationHistory = History;
			return;
		}
	}
	
	ShibNavigationSubsystem->NavigationHistory.Add(FNavigationHistory{ NavigationUniqueId, History });
}

void UShibUiBaseNavigation::ClearGiHistory()
{
	if (!GetShibNavigationSubsystem()) return;
	
	for (int i = 0; i < ShibNavigationSubsystem->NavigationHistory.Num(); i++)
	{
		if (ShibNavigationSubsystem->NavigationHistory[i].UniqueId == NavigationUniqueId)
		{
			ShibNavigationSubsystem->NavigationHistory.RemoveAt(i);
			return;
		}
	}
}

UShibNavigationSubsystem* UShibUiBaseNavigation::GetShibNavigationSubsystem()
{
	if (!ShibNavigationSubsystem && GetGameInstance()) ShibNavigationSubsystem = GetGameInstance()->GetSubsystem<UShibNavigationSubsystem>();
	return ShibNavigationSubsystem;
}

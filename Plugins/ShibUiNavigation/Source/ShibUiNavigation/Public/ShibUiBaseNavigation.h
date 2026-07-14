// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShibUiBaseNavigation.generated.h"

class UCanvasPanel;
class UShibNavigationSubsystem;
class UShibUiBasePage;
class USoundBase;

USTRUCT(BlueprintType)
struct FPageDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName PreviousPageId = FName();
	
	UPROPERTY(BlueprintReadOnly)
	FName PageId = FName();
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibUiBasePage> PageRef = nullptr;

	bool operator==(const FPageDetails& OtherItem) const
	{
		return PageId == OtherItem.PageId;
	}
};

USTRUCT(BlueprintType)
struct FNavigationHistory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName UniqueId = FName();
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FPageDetails> NavigationHistory;
};

UENUM(BlueprintType)
enum ENavigationDirection : uint8
{
	NavForward UMETA(DisplayName = "Forward"),
	NavBackward UMETA(DisplayName = "Backward")
};


UCLASS()
class SHIBUINAVIGATION_API UShibUiBaseNavigation : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/**
	 * Pages used for navigation. {Unique Id, Widget}
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UShibUiBasePage>> Pages;

	/**
	 * Unique Id for this navigation widget.
	 * Used to store multiple history arrays in the game instance.
	 */
	UPROPERTY(EditDefaultsOnly)
	FName NavigationUniqueId;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPageDetails> NavigationHistory;

	UPROPERTY(BlueprintReadOnly)
	FPageDetails CurrentPage;

	/**
	 * Go back to previous page in history array
	 */
	UFUNCTION(BlueprintCallable)
	virtual void GoBack();

	/**
	 * Get the history from the game instance and go to the last page in the history.
	 * If the history is empty, go to the fallback page.
	 * Used for UI navigation between levels.
	 * @param FallbackPageId Page to construct if navigation history is empty.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void GoToLastPageInHistory(const FName FallbackPageId);

	/**
	 * Go to specified page using the navigation system.
	 * @param PageId Page ID to construct
	 * @param bKeepInHistory Keep the page in the history. History is kept as long as we stay in the same level
	 * @param NavigationDirection Navigation direction, hidden from blueprint because this is used internally only
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, meta=(HidePin="NavigationDirection"))
	void GoToPage(const FName PageId, const bool bKeepInHistory = true, ENavigationDirection NavigationDirection=ENavigationDirection::NavForward);

	virtual TArray<FPageDetails> GetHistoryFromGi();
	
	virtual void SetGiHistory(const TArray<FPageDetails>& History);
	
	virtual void ClearGiHistory();
	
	UPROPERTY()
	TObjectPtr<UShibNavigationSubsystem> ShibNavigationSubsystem;
	
	UFUNCTION(BlueprintPure)
	virtual UShibNavigationSubsystem* GetShibNavigationSubsystem();
	
protected:

	// Canvas container where all the other widgets will be added
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasContainer;

	// Time to wait before the previous page is destroyed.
	// Used when changing from one page to another.
	// Time to destroy can differ from one page to another, so this value will change depending on the current page.
	float PrevPageDestructionTime = 0.f;

	FTimerHandle InitPageDurationHandle;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void AddCurrentPageToViewport(ENavigationDirection NavigationDirection);

	virtual void InitCurrentPage(ENavigationDirection NavigationDirection);

	virtual void PopHistory(const FName PopToPageId);
	
	virtual void AddToHistory(const FPageDetails& Page);

	bool IsPageInHistory(const FPageDetails PageToFind);

	bool FindPageInHistory(FName PageIdToFind, FPageDetails& PageFound);
};

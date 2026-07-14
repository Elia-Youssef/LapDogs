// Copyright Shiba Inu Games LLC.

#include "ShibLoadingScreenManager.h"
#include "ShibLoadingProcessInterface.h"
#include "ShibLoadingScreenConfig.h"
#include "Framework/Application/IInputProcessor.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/ThreadHeartBeat.h"
#include "PreLoadScreen.h"
#include "PreLoadScreenManager.h"
#include "ShaderPipelineCache.h"
#include "ShibGameViewportClient.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Images/SThrobber.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ShibLoadingScreenManager)

DECLARE_LOG_CATEGORY_EXTERN(ShibLogLoadingScreen, Log, All);
DEFINE_LOG_CATEGORY(ShibLogLoadingScreen);

//////////////////////////////////////////////////////////////////////

bool IShibLoadingProcessInterface::ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
{
	if (TestObject != nullptr)
	{
		if (IShibLoadingProcessInterface* LoadObserver = Cast<IShibLoadingProcessInterface>(TestObject))
		{
			FString ObserverReason;
			if (LoadObserver->ShouldShowLoadingScreen(/*out*/ ObserverReason))
			{
				if (ensureMsgf(!ObserverReason.IsEmpty(), TEXT("%s failed to set a reason why it wants to show the loading screen"), *GetPathNameSafe(TestObject)))
				{
					OutReason = ObserverReason;
				}
				return true;
			}
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////

namespace LoadingScreenCVars
{
	// CVars
	static float HoldLoadingScreenAdditionalSecs = 2.0f;
	static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
		TEXT("ShibLoadingScreen.HoldLoadingScreenAdditionalSecs"),
		HoldLoadingScreenAdditionalSecs,
		TEXT("How long to hold the loading screen up after other loading finishes (in seconds) to try to give texture streaming a chance to avoid blurriness"),
		ECVF_Default | ECVF_Preview);

	static bool LogLoadingScreenReasonEveryFrame = false;
	static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
		TEXT("ShibLoadingScreen.LogLoadingScreenReasonEveryFrame"),
		LogLoadingScreenReasonEveryFrame,
		TEXT("When true, the reason the loading screen is shown or hidden will be printed to the log every frame."),
		ECVF_Default);

	static bool ForceLoadingScreenVisible = false;
	static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
		TEXT("ShibLoadingScreen.AlwaysShow"),
		ForceLoadingScreenVisible,
		TEXT("Force the loading screen to show."),
		ECVF_Default);
}

//////////////////////////////////////////////////////////////////////
// FShibLoadingScreenInputPreProcessor

// Input processor to throw in when loading screen is shown
// This will capture any inputs, so active menus under the loading screen will not interact
class FShibLoadingScreenInputPreProcessor : public IInputProcessor
{
public:
	FShibLoadingScreenInputPreProcessor() { }
	virtual ~FShibLoadingScreenInputPreProcessor() { }

	bool CanEatInput() const
	{
		return !GIsEditor;
	}

	//~IInputProcess interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override { return CanEatInput(); }
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { return CanEatInput(); }
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanEatInput(); }
	//~End of IInputProcess interface
};

//////////////////////////////////////////////////////////////////////
// ULoadingScreenManager

void UShibLoadingScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	UGameInstance* LocalGameInstance = GetGameInstance();
	check(LocalGameInstance);

	LocalGameInstance->OnNotifyPreClientTravel().AddUObject(this, &ThisClass::HandlePreClientTravel);
}

void UShibLoadingScreenManager::Deinitialize()
{
	StopBlockingInput();

	RemoveWidgetFromViewport();

	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	if (UGameInstance* LocalGameInstance = GetGameInstance())
	{
		LocalGameInstance->OnNotifyPreClientTravel().RemoveAll(this);
	}

	// ** UE5.5 Functionality ONLY**
	// We are done, so do not attempt to tick us again 
	//SetTickableTickType(ETickableTickType::Never);
}

bool UShibLoadingScreenManager::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only clients have loading screens
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();	
	return !bIsServerWorld;
}

void UShibLoadingScreenManager::Tick(float DeltaTime)
{
	UpdateLoadingScreen();

	TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType UShibLoadingScreenManager::GetTickableTickType() const
{
	if (IsTemplate())
	{
		return ETickableTickType::Never;
	}
	return ETickableTickType::Conditional;
}

bool UShibLoadingScreenManager::IsTickable() const
{
	// Don't tick if we don't have a game viewport client, this catches cases that ShouldCreateSubsystem does not
	UGameInstance* GameInstance = GetGameInstance();
	return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId UShibLoadingScreenManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
}

UWorld* UShibLoadingScreenManager::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

void UShibLoadingScreenManager::RegisterLoadingProcessor(TScriptInterface<IShibLoadingProcessInterface> Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void UShibLoadingScreenManager::UnregisterLoadingProcessor(TScriptInterface<IShibLoadingProcessInterface> Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

void UShibLoadingScreenManager::FadeScreen(float Warmuptime, float FadeTime, bool bToBlack)
{
	if (UShibGameViewportClient* ShibGameViewportClient = Cast<UShibGameViewportClient>(GEngine->GameViewport))
	{
		ShibGameViewportClient->Fade(Warmuptime, FadeTime, bToBlack);
	}
	else
	{
		UE_LOG(ShibLogLoadingScreen, Error, TEXT("Fade Screen failed! Make sure the game must use the custom game viewport class ShibGameViewportClient."));
	}
}

void UShibLoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	HandlePreClientTravel(MapName, TRAVEL_Absolute, false);
	
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// Update the loading screen immediately if the engine is initialized
		if (GEngine->IsInitialized())
		{
			UpdateLoadingScreen();
		}
	}
}

void UShibLoadingScreenManager::HandlePostLoadMap(UWorld* World)
{
	if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
	{
		bCurrentlyInLoadMap = false;
	}
	DestinationMapName = ""; // Clear map name for next time
}

void UShibLoadingScreenManager::HandlePreClientTravel(const FString& PendingURL, ETravelType TravelType,
	bool bIsSeamlessTravel)
{
	if (DestinationMapName.IsEmpty()) // If the destination map wasn't pre-filed before, we try to fill it using the URL.
	{
		FString Path, FullName, MapName, Option;
		PendingURL.Split(TEXT("/"), &Path, &FullName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		// With a seamless travel, we have something else to remove after the map name (e.g. "LVL_LapDogs_01_P?Listen")
		if (FullName.Split(TEXT("?"),&MapName, &Option, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
			DestinationMapName = MapName;
		else
			DestinationMapName = FullName;
	}
	
	UE_LOG(ShibLogLoadingScreen, Log, TEXT("Moving to map named: %s"), *DestinationMapName);
}

void UShibLoadingScreenManager::UpdateLoadingScreen()
{
	bool bLogLoadingScreenStatus = LoadingScreenCVars::LogLoadingScreenReasonEveryFrame;

	if (ShouldShowLoadingScreen())
	{
		const auto* Settings = UShibLoadingScreenConfig::Get();
		
		// If we don't make it to the specified checkpoint in the given time will trigger the hang detector so we can better determine where progress stalled.
		FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), Settings->LoadingScreenHeartbeatHangDuration);

		ShowLoadingScreen();

		if ((Settings->LogLoadingScreenHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
		{
			bLogLoadingScreenStatus = true;
			TimeUntilNextLogHeartbeatSeconds = Settings->LogLoadingScreenHeartbeatInterval;
		}
	}
	else
	{
		HideLoadingScreen();
 
		FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
	}

	if (bLogLoadingScreenStatus)
	{
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingLoadingScreen ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
	}
}

bool UShibLoadingScreenManager::CheckForAnyNeedToShowLoadingScreen()
{
	// Start out with 'unknown' reason in case someone forgets to put a reason when changing this in the future.
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("Reason for Showing/Hiding LoadingScreen is unknown!");

	const UGameInstance* LocalGameInstance = GetGameInstance();

	if (LoadingScreenCVars::ForceLoadingScreenVisible)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommonLoadingScreen.AlwaysShow is true"));
		return true;
	}

	const FWorldContext* Context = LocalGameInstance->GetWorldContext();
	if (Context == nullptr)
	{
		// We don't have a world context right now... better show a loading screen
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
		return true;
	}

	UWorld* World = Context->World();
	if (World == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
		return true;
	}

	AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
	if (GameState == nullptr)
	{
		// The game state has not yet replicated.
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
		return true;
	}

	if (bCurrentlyInLoadMap)
	{
		// Show a loading screen if we are in LoadMap
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
		return true;
	}

	if (!Context->TravelURL.IsEmpty())
	{
		// Show a loading screen when pending travel
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
		return true;
	}

	if (Context->PendingNetGame != nullptr)
	{
		// Connecting to another server
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
		return true;
	}

	if (!World->HasBegunPlay())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
		return true;
	}

	if (World->IsInSeamlessTravel())
	{
		// Show a loading screen during seamless travel
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
		return true;
	}

	// Ask the game state if it needs a loading screen	
	if (IShibLoadingProcessInterface::ShouldShowLoadingScreen(GameState, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
	{
		return true;
	}

	// Ask any game state components if they need a loading screen
	for (UActorComponent* TestComponent : GameState->GetComponents())
	{
		if (IShibLoadingProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// Ask any of the external loading processors that may have been registered.  These might be actors or components
	// that were registered by game code to tell us to keep the loading screen up while perhaps something finishes
	// streaming in.
	for (const TWeakInterfacePtr<IShibLoadingProcessInterface>& Processor : ExternalLoadingProcessors)
	{
		if (IShibLoadingProcessInterface::ShouldShowLoadingScreen(Processor.GetObject(), /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// Check each local player
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// Ask the PC itself if it needs a loading screen
				if (IShibLoadingProcessInterface::ShouldShowLoadingScreen(PC, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
				{
					return true;
				}

				// Ask any PC components if they need a loading screen
				for (UActorComponent* TestComponent : PC->GetComponents())
				{
					if (IShibLoadingProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
					{
						return true;
					}
				}
			}
			else
			{
				bMissingAnyLocalPC = true;
			}
		}
	}

	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
	const bool bIsInSplitscreen = GameViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;

	// In splitscreen we need all player controllers to be present
	if (bIsInSplitscreen && bMissingAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
		return true;
	}

	// And in non-splitscreen we need at least one player controller to be present
	if (!bIsInSplitscreen && !bFoundAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
		return true;
	}

	// Victory! The loading screen can go away now
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool UShibLoadingScreenManager::ShouldShowLoadingScreen()
{
	const auto* Settings = UShibLoadingScreenConfig::Get();

	// Check debugging commands that force the state one way or another
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
		return false;
	}
#endif

	// Can't show a loading screen if there's no game viewport
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LocalGameInstance->GetGameViewportClient() == nullptr) return false;

	// Check for a need to show the loading screen
	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

	// Keep the loading screen up a bit longer if desired
	bool bWantToForceShowLoadingScreen = false;
	if (bNeedToShowLoadingScreen)
	{
		// Still need to show it
		TimeLoadingScreenLastDismissed = -1.0;
	}
	else
	{
		// Don't *need* to show the screen anymore, but might still want to for a bit
		const double CurrentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
		const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeLoadingScreenLastDismissed < 0.0)
		{
			TimeLoadingScreenLastDismissed = CurrentTime;
		}
		const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

		// hold for an extra X seconds, to cover up streaming
		if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
		{
			// Make sure we're rendering the world at this point, so that textures will actually stream in
			//@TODO: If bNeedToShowLoadingScreen bounces back true during this window, we won't turn this off again...
			UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
			GameViewportClient->bDisableWorldRendering = false;

			DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), HoldLoadingScreenAdditionalSecs);
			bWantToForceShowLoadingScreen = true;
		}
	}

	return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool UShibLoadingScreenManager::IsShowingInitialLoadingScreen() const
{
	FPreLoadScreenManager* PreLoadScreenManager = FPreLoadScreenManager::Get();
	return (PreLoadScreenManager != nullptr) && PreLoadScreenManager->HasValidActivePreLoadScreen();
}

void UShibLoadingScreenManager::ShowLoadingScreen()
{
	if (bCurrentlyShowingLoadingScreen) return;

	// Unable to show loading screen if the engine is still loading with its loading screen.
	if (FPreLoadScreenManager::Get() && FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen)) return;

	TimeLoadingScreenShown = FPlatformTime::Seconds();

	bCurrentlyShowingLoadingScreen = true;

	//CSV_EVENT(LoadingScreen, TEXT("Show"));

	const auto* Settings = UShibLoadingScreenConfig::Get();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UGameInstance* LocalGameInstance = GetGameInstance();
		
		// Eat input while the loading screen is displayed
		StartBlockingInput();

		LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ true);

		// Create the loading screen widget
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass = Settings->LoadingScreenWidget.TryLoadClass<UUserWidget>();
		if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
		{
			LoadingScreenWidget = UserWidget->TakeWidget();
		}
		else
		{
			UE_LOG(ShibLogLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
			LoadingScreenWidget = SNew(SThrobber);
		}

		// Add to the viewport at a high ZOrder to make sure it is on top of most things
		UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
		GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);

		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ true);

		if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
		{
			// Tick Slate to make sure the loading screen is displayed immediately
			FSlateApplication::Get().Tick();
		}
	}
}

void UShibLoadingScreenManager::HideLoadingScreen()
{
	if (!bCurrentlyShowingLoadingScreen) return;
	
	if (IsShowingInitialLoadingScreen())
	{
		StopBlockingInput();
		
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
		
		//CSV_EVENT(LoadingScreen, TEXT("Hide"));
	
		const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
		UE_LOG(ShibLogLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);
	}
	else
	{
		const auto* Settings = UShibLoadingScreenConfig::Get();
		FadeScreen(Settings->FadeWarmupDuration,Settings->FadeDuration,true);
		
		// Wait for the fade to finish before letting everyone knows that the map is loaded
		FTimerHandle FadeIsOverHandle;
		GetWorld()->GetTimerManager().SetTimer(FadeIsOverHandle, [&]()
		{
			const auto* Settings = UShibLoadingScreenConfig::Get();
			FadeScreen(Settings->FadeWarmupDuration,Settings->FadeDuration,false);
			
			StopBlockingInput();
			
			UE_LOG(ShibLogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
			UE_LOG(ShibLogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

			UE_LOG(ShibLogLoadingScreen, Log, TEXT("Garbage Collecting before dropping load screen"));
			GEngine->ForceGarbageCollection(true);
				
			RemoveWidgetFromViewport();
		
			ChangePerformanceSettings(/*bEnableLoadingScreen=*/ false);
	
			// Let observers know that the loading screen is done
			LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ false);
			
			//CSV_EVENT(LoadingScreen, TEXT("Hide"));
		
			const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
			UE_LOG(ShibLogLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);
		}, Settings->FadeWarmupDuration+Settings->FadeDuration, false);
	}
	
	// Reset Destination map name because loading phase is done
	DestinationMapName = FString();

	bCurrentlyShowingLoadingScreen = false;
}

void UShibLoadingScreenManager::RemoveWidgetFromViewport()
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LoadingScreenWidget.IsValid())
	{
		if (UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient())
		{
			GameViewportClient->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
		}
		LoadingScreenWidget.Reset();
	}
}

void UShibLoadingScreenManager::StartBlockingInput()
{
	if (!InputPreProcessor.IsValid())
	{
		InputPreProcessor = MakeShareable<FShibLoadingScreenInputPreProcessor>(new FShibLoadingScreenInputPreProcessor());
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void UShibLoadingScreenManager::StopBlockingInput()
{
	if (InputPreProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		InputPreProcessor.Reset();
	}
}

void UShibLoadingScreenManager::ChangePerformanceSettings(bool bEnablingLoadingScreen)
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();

	FShaderPipelineCache::SetBatchMode(bEnablingLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

	// Don't bother drawing the 3D world while we're loading
	GameViewportClient->bDisableWorldRendering = bEnablingLoadingScreen;

	// Make sure to prioritize streaming in levels if the loading screen is up
	if (UWorld* ViewportWorld = GameViewportClient->GetWorld())
	{
		if (AWorldSettings* WorldSettings = ViewportWorld->GetWorldSettings(false, false))
		{
			WorldSettings->bHighPriorityLoadingLocal = bEnablingLoadingScreen;
		}
	}

	if (bEnablingLoadingScreen)
	{
		// Set a new hang detector timeout multiplier when the loading screen is visible.
		double HangDurationMultiplier;
		if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), /*out*/ HangDurationMultiplier, GEngineIni))
		{
			HangDurationMultiplier = 1.0;
		}
		FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);

		// Do not report hitches while the loading screen is up
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// Restore the hang detector timeout when we hide the loading screen
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		// Resume reporting hitches now that the loading screen is down
		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
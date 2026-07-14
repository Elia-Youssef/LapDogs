// Fill out your copyright notice in the Description page of Project Settings.

#include "ShibSessionsEOS.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"
#include "ShibUserEOS.h"
#include "Online/OnlineSessionNames.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibSessionSubsystem, Log, All);
DEFINE_LOG_CATEGORY(LogShibSessionSubsystem);

UShibSessionsEOS::UShibSessionsEOS() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleFindSessionsComplete)),
	CancelFindSessionsCompleteDelegate(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleCancelFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleDestroySessionComplete)),
	EndSessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleEndSessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleStartSessionComplete))
{
}

void UShibSessionsEOS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	checkf(Subsystem != nullptr, TEXT("Unable to get Online Subsystem."));

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	bCreateSessionOnDestroy = false;
}

void UShibSessionsEOS::Deinitialize()
{
	Super::Deinitialize();
}

void UShibSessionsEOS::CreateSession(const int32 NumConnections, const FString& SessionName,
                                     const bool bIsDedicatedServer, const bool bIsPrivate, const bool bIsLANMatch,
                                     const TMap<FName, FString>& CustomSessionSettings)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	LastCustomSessionSettings = CustomSessionSettings;

	// Save session params
	LastNumPublicConnections = NumConnections;
	LastSessionName = SessionName;
	bLastSessionIsDedicatedServer = bIsDedicatedServer;
	bLastSessionIsPrivate = bIsPrivate;
	bLastSessionIsLAN = bIsLANMatch;

	//Destroy existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		DestroySession();
		return;
	}

	//Register the delegate for when the creation complete and store its handle for later removal
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//Create the session parameters. We could not use the MakeShareable and thus the new
	//but this way we can make SessionSettings a member of our class for future reuse.
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

	LastSessionSettings->bIsDedicated = bIsDedicatedServer;
	LastSessionSettings->bIsLANMatch = bIsLANMatch;
	LastSessionSettings->NumPublicConnections = NumConnections;
	//LastSessionSettings->bAllowInvites = true;
	//LastSessionSettings->bUseLobbiesIfAvailable = false;
	LastSessionSettings->bAllowJoinInProgress = false;
	//LastSessionSettings->bAllowJoinViaPresence = false;
	//LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	LastSessionSettings->bShouldAdvertise = true;
	//LastSessionSettings->bUsesPresence = false;
	//LastSessionSettings->Set(FName("IsPrivate"), bIsPrivate, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(SEARCH_KEYWORDS, FString("RandomHi"), EOnlineDataAdvertisementType::ViaOnlineService);

	for (auto [Key, Value] : CustomSessionSettings) {
		LastSessionSettings->Set(Key, Value, EOnlineDataAdvertisementType::ViaOnlineService);
	}

#if !UE_BUILD_SHIPPING
	//Enforce a specific Build ID in not shipping so we can
	//easily test session creation
	LastSessionSettings->BuildUniqueId = 1;
#endif

	//Create the session
	const bool success = SessionInterface->CreateSession(0, ShibGameSession, *LastSessionSettings);
	if (!success)
	{
		//We failed to create the session simply remove the delegate for completion
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::FindSessions(const int32 MaxSearchResults, const bool bIsLANMatch,
                                    const TMap<FName, FString>& CustomSearchSettings)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bIsLANMatch;
	
	// Add your search settings here...
	// Add this dummy search filter if you don't have any. You must include at least one search filter or EOS will not return any search results.
	//LastSessionSearch->QuerySettings.Set(FName(TEXT("SessionSetting")), FString(TEXT("SettingValue")), EOnlineComparisonOp::Equals);
	
	//To search for Presence session only, add the following search filter:
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	//To search for only non-listening sessions, add the following search filter:
	//LastSessionSearch->QuerySettings.Set(FName(TEXT("__EOS_bListening")), true, EOnlineComparisonOp::Equals);
	//To search for both listening and non-listening sessions, add the following search filter:
	//LastSessionSearch->QuerySettings.Set(FName(TEXT("minslotsavailable")), FVariantData((int64)1), EOnlineComparisonOp::GreaterThanEquals);
	// To include sessions which are full, add the following search filter:
	//LastSessionSearch->QuerySettings.SearchParams.Add(FName(TEXT("minslotsavailable")), FOnlineSessionSearchParam((int64)0L, EOnlineComparisonOp::GreaterThanEquals));
	// To search for a specific match type, add the following search filter:
	//LastSessionSearch->QuerySettings.Set(FName("MatchType"), MatchType, EOnlineComparisonOp::Equals);
	
	for (auto [Key, Value] : CustomSearchSettings) {
		LastSessionSearch->QuerySettings.Set(Key, Value, EOnlineComparisonOp::Equals);
	}
	
	//Register the delegate for when the find session complete and store its handle for later removal
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
																												    
	//Get the player's Unique Net ID from our user subsystem
	auto* GameInstance = GetGameInstance();
	auto* UserSubsystem = GameInstance->GetSubsystem<UShibUserEOS>();

	const bool success = SessionInterface->FindSessions(*UserSubsystem->GetNetID().GetUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!success)
	{
		//We failed to find sessions simply remove the delegate for completion
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();

		TArray<FBlueprintSessionResult> Results;
		OnFindSessionsComplete.Broadcast(Results, false);
	}
}

void UShibSessionsEOS::CancelFindSessions()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	CancelFindSessionsCompleteDelegateHandle = SessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegate);

	if (const bool success = SessionInterface->CancelFindSessions(); !success)
	{
		SessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegateHandle);
		CancelFindSessionsCompleteDelegateHandle.Reset();
		
		OnCancelFindSessionsComplete.Broadcast(false);
	}
}
void UShibSessionsEOS::JoinSession(const FBlueprintSessionResult& SessionResult)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;
	
	//Register the delegate for when the join session complete and store its handle for later removal
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	//Get the player's Unique Net ID from our user subsystem
	auto* GameInstance = GetGameInstance();
	auto* UserSubsystem = GameInstance->GetSubsystem<UShibUserEOS>();

	const bool success = SessionInterface->JoinSession(*UserSubsystem->GetNetID().GetUniqueNetId(), ShibGameSession, SessionResult.OnlineResult);
	if (!success)
	{
		//We failed to join session simply remove the delegate for completion
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		OnJoinSessionComplete.Broadcast(false, EShibJoinSessionResultTypeEOS::UnknownErrorEOS, "");
	}
}

void UShibSessionsEOS::StartSession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(ShibGameSession))
	{
		//We failed to Start session simply remove the delegate for completion
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
		OnStartSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::EndSession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	EndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);

	//End existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->EndSession(ShibGameSession);
	}
	else //No session found
	{
		//Remove the end session completion delegate
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();

		OnEndSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::DestroySession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	//Destroy existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(ShibGameSession);
	}
	else //No session found
	{
		//Remove the destroy session completion delegate
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		OnDestroySessionComplete.Broadcast(false);
	}
}

bool UShibSessionsEOS::RegisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	const auto* Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	
	// Register the player with the session name; this name should match the name you provided in CreateSession.
	bool RegisterResult = SessionInterface->RegisterPlayer(ShibGameSession, *UniqueNetId, false);
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Player %s registered: %s"), *UniqueNetId.Get()->ToString(), RegisterResult ? TEXT("true") : TEXT("false"));
	
	return RegisterResult;
}

bool UShibSessionsEOS::RegisterExistingPlayers()
{
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		// if (PlayerController->HasAuthority()) continue;

		// Register players, stop execution and return false if one of them has not successfully registered.
		if (!RegisterPlayer(PlayerController)) return false;
	}

	return true;
}

bool UShibSessionsEOS::UnregisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	const auto* Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	// Unregister the player with the session name; this name should match the name you provided in CreateSession.
	bool UnregisterResult = SessionInterface->UnregisterPlayer(ShibGameSession, *UniqueNetId);
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Player %s unregistered: %s"), *UniqueNetId.Get()->ToString(), UnregisterResult ? TEXT("true") : TEXT("false"));
	
	return UnregisterResult;
}

void UShibSessionsEOS::CleanUpSessions()
{
	// Get the online session interface. 
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	//Destroy existing session locally if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);

	if (ExistingSession != nullptr)
	{
		DestroySession();
	}
}

void UShibSessionsEOS::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s created: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleFindSessionsComplete(bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();
	}

	if (!LastSessionSearch.IsValid() || LastSessionSearch->SearchResults.Num() <= 0)
	{
		TArray<FBlueprintSessionResult> Results;
		UE_LOG(LogShibSessionSubsystem, Log, TEXT("Sessions found: false"));
		OnFindSessionsComplete.Broadcast(Results, false);
		return;
	}

	TArray<FBlueprintSessionResult> Results;
	Results.Reserve(LastSessionSearch->SearchResults.Num());

	for (const auto& result : LastSessionSearch->SearchResults)
	{
		if (result.IsValid())
		{
			FBlueprintSessionResult r{};
			r.OnlineResult = result;
			Results.Add(r);
		}
	}

	if (Results.Num() != 0)
	{
		UE_LOG(LogShibSessionSubsystem, Log, TEXT("Sessions found: true"));
	}
	else
	{
		UE_LOG(LogShibSessionSubsystem, Log, TEXT("Sessions found: false"));
	}
	
	OnFindSessionsComplete.Broadcast(Results, true);
}

void UShibSessionsEOS::HandleCancelFindSessionsComplete(bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegateHandle);
		CancelFindSessionsCompleteDelegateHandle.Reset();
	}

	OnCancelFindSessionsComplete.Broadcast(true);
}

void UShibSessionsEOS::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s joinned: %s"), *SessionName.ToString(), Result == EOnJoinSessionCompleteResult::Type::Success ? TEXT("true") : TEXT("false"));

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		// Add network error handle if the game client fails to connect to the server
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &UShibSessionsEOS::HandleNetworkFailure);
	}
	else
	{
		OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, UnknownErrorEOS, "");
		return;
	}

	EShibJoinSessionResultTypeEOS ResultBP = EShibJoinSessionResultTypeEOS::UnknownErrorEOS;
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		ResultBP = ResultSuccessEOS;
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ResultBP = SessionIsFullEOS;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ResultBP = SessionDoesNotExistEOS;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ResultBP = CouldNotRetrieveAddressEOS;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ResultBP = AlreadyInSessionEOS;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		ResultBP = UnknownErrorEOS;
		break;
	}

	//Get the session address to make a client travel
	FString Address;
	SessionInterface->GetResolvedConnectString(ShibGameSession, Address);
	
	//Fire our own delegate
	OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, ResultBP, Address);
}

void UShibSessionsEOS::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		// Unregister network failure handler when we destroy the current session
		if (NetworkFailureDelegateHandle.IsValid())
		{
			GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
			NetworkFailureDelegateHandle.Reset();
		}
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s destroyed: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnDestroySessionComplete.Broadcast(bWasSuccessful);

	// Check if we need to create another session right after it got destroyed
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false; // Make sure to set this back to false to not get stuck in a loop
		CreateSession(LastNumPublicConnections, LastSessionName, bLastSessionIsDedicatedServer, bLastSessionIsPrivate, bLastSessionIsLAN, LastCustomSessionSettings);
	}
}

void UShibSessionsEOS::HandleEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s ended: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnEndSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s started: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		// Optional: Notify players or handle reconnection logic
	}

	// Unregister network failure handler
	GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	NetworkFailureDelegateHandle.Reset();
}

#include "ShibMatchmakingEOS.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibMatchmakingSubsystem, Log, All);
DEFINE_LOG_CATEGORY(LogShibMatchmakingSubsystem);

UShibMatchmakingEOS::UShibMatchmakingEOS()
{
    this->bIsMatchmaking=false;
    this->LastMatchmakingProgressData={};
}

void UShibMatchmakingEOS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UShibMatchmakingEOS::Deinitialize()
{
	Super::Deinitialize();
}

void UShibMatchmakingEOS::StartMatchmaking(FShibMatchmakerHostConfiguration MatchMakingHostConfig)
{
    if (!MatchMakingHostConfig.bUseDedicatedServers && MatchMakingHostConfig.Map.IsNull())
    {
        UE_LOG(LogShibMatchmakingSubsystem, Error, TEXT("Can't start matchmaking because the configured map is invalid!"));
        return;
    }

#if WITH_EDITOR
    // Try to make the request ID extra unique when running in the editor.
    int64 RequestId = (FDateTime::UtcNow().GetTicks() / 100000) * 100000;
    if (GEngine != nullptr && IsValid(this->GetWorld()))
    {
        FWorldContext &WorldContext = GEngine->GetWorldContextFromWorldChecked(this->GetWorld());
        RequestId += ((int64)WorldContext.PIEInstance % 100) * 1000;
    }
    RequestId += FMath::RandRange(0, 999);
#else
    int64 RequestId = FDateTime::UtcNow().GetTicks();
#endif
    
    auto* Subsystem = Online::GetSubsystem(this->GetWorld());

    FMatchmakingEngineRequest Request = {};
    // You need to pass the identity, lobby, party and session interfaces to the request.
    // This is so that the matchmaker can access the relevant subsystem.
    Request.Identity = Subsystem->GetIdentityInterface();
    Request.Lobby = Online::GetLobbyInterface(Subsystem);
    Request.PartySystem = Subsystem->GetPartyInterface();
    Request.Session = Subsystem->GetSessionInterface();
    // Pass the world context handle. This is so the matchmaker can identify which "play-in-editor"
    // instance this matchmaking request is occurring in.
    Request.WorldContextHandle = GEngine->GetWorldContextFromWorldChecked(this->GetWorld()).ContextHandle;
    // Pass the user ID of the player that is queuing up. The code below just gets the user ID
    // of the first local player controller.
    // NOTE: The code below assumes the local player is signed in (or has been validated as
    // signed in). If you're not already checking that, you'll need to add nullptr checks.
    Request.UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0).ToSharedRef();
    // Pass the party ID, if the player is queuing in a party. You can pass `nullptr` here if
    // the user is queuing solo.
    Request.PartyId = nullptr;

    // Cache host config
    CachedHostConfig = MatchMakingHostConfig;
    
    auto RequestHostConfiguration = MakeShared<FMatchmakingEngineRequestHostConfiguration>();
    RequestHostConfiguration->RequestId = RequestId;

    // ======== Main settings we care about for host config =========
    RequestHostConfiguration->QueueName = MatchMakingHostConfig.QueueName.IsEmpty() ? TEXT("Default") : MatchMakingHostConfig.QueueName;// TEXT("Default");

    // Parse team capacities
    TArray<int32> GeneratedTeamCapacities;
    if (MatchMakingHostConfig.TeamCapacities.Contains(TEXT("x")))
    {
        TArray<FString> MultiplyComponents;
        MatchMakingHostConfig.TeamCapacities.ParseIntoArray(MultiplyComponents, TEXT("x"));
        if (MultiplyComponents.Num() != 2)
        {
            UE_LOG(
                LogShibMatchmakingSubsystem,
                Error,
                TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return;
        }
        int32 X = FCString::Atoi(*MultiplyComponents[0]);
        int32 Y = FCString::Atoi(*MultiplyComponents[1]);
        if (X <= 0 || Y <= 0)
        {
            UE_LOG(
                LogShibMatchmakingSubsystem,
                Error,
                TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return;
        }
        for (int i = 0; i < Y; i++)
        {
            GeneratedTeamCapacities.Add(X);
        }
    }
    else
    {
        TArray<FString> TeamComponents;
        MatchMakingHostConfig.TeamCapacities.ParseIntoArray(TeamComponents, TEXT("v"));
        if (TeamComponents.Num() == 0)
        {
            UE_LOG(
                LogShibMatchmakingSubsystem,
                Error,
                TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return;
        }
        for (const auto &CapacityString : TeamComponents)
        {
            int32 Capacity = FCString::Atoi(*CapacityString);
            if (Capacity == 0)
            {
                UE_LOG(LogShibMatchmakingSubsystem, Warning, TEXT("Team capacity of 0 will be ignored!"));
                continue;
            }
            GeneratedTeamCapacities.Add(Capacity);
        }
    }
    
    RequestHostConfiguration->TeamCapacities = GeneratedTeamCapacities;
    
    // When matchmaking completes, what should the matchmaker do? There are currently three
    // supported options and clients and hosts must have the same setting:
    // - ReturnResults: OnComplete is fired with the results.
    // - StartListenServerWithMap: The matchmaker will start a multiplayer listen server and
    //   get players to join it.
    // - FindDedicatedServer: The matchmaker will search for a dedicated server to play on and
    //   get players to join it.
    if (MatchMakingHostConfig.bUseDedicatedServers)
    {
        Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::FindDedicatedServer; 
    }
    else
    {
        Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap;
    }
    
    // For StartListenServerWithMap and FindDedicatedServer, this indicates whether 
    // players should join via the session ID or directly via the connection URL.
    Request.CompletionBehaviour.bConnectViaSessionID = !Subsystem->GetSubsystemName().IsEqual(STEAM_SUBSYSTEM);
    
    // ======== Timing and balance settings not available in blueprints =========
    // When we run out of time, and there's no more players online to bring into our match, 
    // what should the matchmaker do? Available options are:
    // - EMatchmakingBehaviourOnNoCandidates::WaitUntilFull: Ignore the timeouts and continue
    //   searching indefinitely. If your game doesn't work with partially filled or unbalanced
    //   teams, this is the option to pick. Note however that "estimated time to completion"
    //   will be inaccurate once the timeout has run out, since the players could be waiting
    //   forever if no-one else comes online.
    // - EMatchmakingBehaviourOnNoCandidates::CompletePartiallyFilled: The match will be completed
    //   with the remaining slots set to "Empty".
    // - EMatchmakingBehaviourOnNoCandidates::CompleteFillWithAI: The match will be completed
    //   with the remaining slots set to "AI". This is currently the same behaviour as 
    //   CompletePartiallyFilled but we might spawn AI controllers for you in future if the the
    //   matchmaking engine is also starting a listen server.

    // The timeout for matchmaking is calculated as:
    //   baseline seconds + (remaining slots * per slot seconds)
    // Typically that means the more empty the match, the longer we're willing to wait for
    // more players to be online.
    //
    // If you set these settings too low, the matchmaker won't have enough time to find 
    // players and you'll get a large number of partially filled matches.
    if (MatchMakingHostConfig.bAllowPartiallyFilledMatches)
    {
        RequestHostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::CompleteFillWithAI;
        RequestHostConfiguration->BalanceMode = MatchMakingHostConfig.bPrioritizeBalance ? EMatchmakingBalanceMode::MaximizeBalance : EMatchmakingBalanceMode::MaximizeTeamFill;
        RequestHostConfiguration->MinimumWaitSecondsBaseline = MatchMakingHostConfig.NoCandidatesTimeout;
        RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = MatchMakingHostConfig.NoCandidatesTimeoutPerEmptySlot;
        if (RequestHostConfiguration->MinimumWaitSecondsBaseline < 15)
        {
            RequestHostConfiguration->MinimumWaitSecondsBaseline = 15;
        }
        if (RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot < 0)
        {
            RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = 0;
        }
    }
    else
    {
        RequestHostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::WaitUntilFull;
        RequestHostConfiguration->BalanceMode = EMatchmakingBalanceMode::MaximizeTeamFill;
        // These settings don't actually do anything in WaitUntilFull mode.
        RequestHostConfiguration->MinimumWaitSecondsBaseline = 60;
        RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = 5;
    }
    
    RequestHostConfiguration->SkillStatPrefix = MatchMakingHostConfig.SkillStatPrefix;
    RequestHostConfiguration->SkillThresholdFunction.Unbind(); // Leave default.
    
    if (MatchMakingHostConfig.bUseDedicatedServers)
    {
        // For FindDedicatedServer, this is the port the dedicated server beacon is listening on.
        // This value defaults to 9990, which is the default port.
        Request.CompletionBehaviour.DedicatedServerBeaconPort = MatchMakingHostConfig.BeaconPort; //FName(TEXT("9990")); //NAME_None

        // For FindDedicatedServer, this callback is used to get the session search parameters when
        // the matchmaker is looking for dedicated servers to play on.
        Request.CompletionBehaviour.OnGetDedicatedServerSearchParams = FMatchmakingEngineGetDedicatedServerSearchParams::CreateUObject(this, &UShibMatchmakingEOS::OnHandleGetDedicatedServerSearchParams);
        // =============================================
    }
    else
    {
        FString LongPackageMapName =  MatchMakingHostConfig.Map.GetLongPackageName();
        Request.CompletionBehaviour.MapName = FName(*LongPackageMapName);; //FName(TEXT("/Game/Maps/MenuLevels/Lvl_LobbyScreenLevel"));
    }
    
    // As a host. If the player is queuing solo, or is the leader of the party, we need to set HostConfiguration.
    // As a client. If the player is in a party, but is not the leader, HostConfiguration should be 'nullptr'. 
    Request.HostConfiguration = RequestHostConfiguration;
    
    Request.CompletionBehaviour.OnResultsReady =
        FMatchmakingEngineRequestComplete::CreateUObject(this, &UShibMatchmakingEOS::OnHandleResultsReady);
    Request.CompletionBehaviour.OnComplete =
        FMatchmakingEngineRequestComplete::CreateUObject(this, &UShibMatchmakingEOS::OnHandleComplete);
    Request.OnProgress =
        FMatchmakingEngineRequestProgress::CreateUObject(this, &UShibMatchmakingEOS::OnHandleProgress);
    Request.OnCancelled =
        FMatchmakingEngineRequestCancelled::CreateUObject(this, &UShibMatchmakingEOS::OnHandleCancelled);
    Request.OnError = FMatchmakingEngineRequestError::CreateUObject(this, &UShibMatchmakingEOS::OnHandleError);
    
    // Queue up the request! The returned handle allows you to cancel the matchmaking request later.
    this->Handle = IMatchmakingEngine::Get()->Enqueue(Request);
    this->bIsMatchmaking=true;
}

void UShibMatchmakingEOS::CancelMatchmaking()
{
    if (this->bIsMatchmaking)
    {
        IMatchmakingEngine::Get()->Cancel(this->Handle);
    }
}

void UShibMatchmakingEOS::IsMatchmaking(bool& bOutIsMatchmaking)
{
    bOutIsMatchmaking = bIsMatchmaking;
}

void UShibMatchmakingEOS::GetMatchmakingStatus(FShibMatchmakerProgressInfo& ProgressInfo) const
{
    if (this->bIsMatchmaking)
    {
        ProgressInfo = this->LastMatchmakingProgressData;
    }
    else
    {
        ProgressInfo = FShibMatchmakerProgressInfo();
        ProgressInfo.StepName = NAME_None;
        ProgressInfo.CurrentStatus = FText::GetEmpty();
        ProgressInfo.CurrentDetail = FText::GetEmpty();
        ProgressInfo.CurrentProgress = 0;
        ProgressInfo.EstimatedTimeOfCompletion = FDateTime::MinValue();
    }
}

void UShibMatchmakingEOS::OnHandleProgress(const FString& TaskId, const FMatchmakingEngineRequest& Request,
                                           FMatchmakingEngineProgressInfo ProgressInfo)
{
    LastMatchmakingProgressData.StepName = ProgressInfo.StepName;
    LastMatchmakingProgressData.CurrentStatus = ProgressInfo.CurrentStatus;
    LastMatchmakingProgressData.CurrentDetail = ProgressInfo.CurrentDetail;
    LastMatchmakingProgressData.CurrentProgress = ProgressInfo.CurrentProgress;
    LastMatchmakingProgressData.EstimatedTimeOfCompletion = ProgressInfo.EstimatedTimeOfCompletion;
    
    this->OnProgress.Broadcast(
    ProgressInfo.StepName,
    ProgressInfo.CurrentStatus,
    ProgressInfo.CurrentDetail,
    ProgressInfo.CurrentProgress,
    ProgressInfo.EstimatedTimeOfCompletion);
}

void UShibMatchmakingEOS::OnHandleResultsReady(const FString& TaskId, const FMatchmakingEngineRequest& Request,
	FMatchmakingEngineResponse Response)
{
}

void UShibMatchmakingEOS::OnHandleComplete(const FString& TaskId, const FMatchmakingEngineRequest& Request,
	FMatchmakingEngineResponse Response)
{
    this->bIsMatchmaking=false;
    
    TArray<FString> Results;
    for (int32 t = 0; t < Response.Teams.Num(); t++)
    {
        if (Results.Num() != 0)
        {
            Results.Add(TEXT(""));
        }
        Results.Add(FString::Printf(TEXT("== Team %d =="), t));
        for (int32 s = 0; s < Response.Teams[t].Slots.Num(); s++)
        {
            switch (Response.Teams[t].Slots[s].Type)
            {
            case EMatchmakingEngineResponseTeamSlotType::Empty:
                Results.Add(FString::Printf(TEXT("Slot %d: (none)"), s));
                break;
            case EMatchmakingEngineResponseTeamSlotType::AI:
                Results.Add(FString::Printf(TEXT("Slot %d: (AI)"), s));
                break;
            case EMatchmakingEngineResponseTeamSlotType::User:
                Results.Add(FString::Printf(TEXT("Slot %d: %s"), s, *Response.Teams[t].Slots[s].UserId->ToString()));
                break;
            }
        }
    }

    // Create request info, we don't completely fill everything here because this isn't necessary. We can do it in the future...
    // Team Capacities (We have this info in the variable TeamResults), Map, Session filters and Beacon port are left empty
    FShibMatchmakerHostConfiguration RequestConfig;
    if (Request.HostConfiguration.Get())
    {
        RequestConfig.QueueName = Request.HostConfiguration->QueueName;
        RequestConfig.bAllowPartiallyFilledMatches = Request.HostConfiguration->OnNoCandidates!=EMatchmakingBehaviourOnNoCandidates::WaitUntilFull;
        RequestConfig.NoCandidatesTimeout = Request.HostConfiguration->MinimumWaitSecondsBaseline;
        RequestConfig.NoCandidatesTimeoutPerEmptySlot = Request.HostConfiguration->MinimumWaitSecondsPerEmptySlot;
        RequestConfig.bPrioritizeBalance = Request.HostConfiguration->BalanceMode==EMatchmakingBalanceMode::MaximizeBalance;
        RequestConfig.SkillStatPrefix = Request.HostConfiguration->SkillStatPrefix;
        RequestConfig.bUseDedicatedServers = Request.CompletionBehaviour.Type==EMatchmakingEngineCompletionBehaviourType::FindDedicatedServer;
    }
    
    this->OnComplete.Broadcast(FString::Join(Results, TEXT("\n")), RequestConfig);
}

void UShibMatchmakingEOS::OnHandleCancelled(const FString& TaskId, const FMatchmakingEngineRequest& Request)
{
    this->bIsMatchmaking=false;
    this->OnCancelled.Broadcast();
}

void UShibMatchmakingEOS::OnHandleError(const FString& TaskId, const FMatchmakingEngineRequest& Request,
	const FOnlineError& Error)
{
    this->bIsMatchmaking=false;
    this->OnError.Broadcast();
}

FSearchParams UShibMatchmakingEOS::OnHandleGetDedicatedServerSearchParams(const FString& TaskId,
    const FMatchmakingEngineRequest& Request, FMatchmakingEngineResponse Response, int32 SearchIteration)
{
    FSearchParams Params;
    for (const auto &Filter : this->CachedHostConfig.SessionFilters)
    {
        EOnlineComparisonOp::Type Comparison = EOnlineComparisonOp::Equals;
        switch (Filter.Comparison)
        {
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::Equal:
            Comparison = EOnlineComparisonOp::Equals;
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::NotEqual:
            Comparison = EOnlineComparisonOp::NotEquals;
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::LessThan:
            Comparison = EOnlineComparisonOp::LessThan;
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::LessThanOrEqual:
            Comparison = EOnlineComparisonOp::LessThanEquals;
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::GreaterThan:
            Comparison = EOnlineComparisonOp::GreaterThan;
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterComparison::GreaterThanOrEqual:
            Comparison = EOnlineComparisonOp::GreaterThanEquals;
            break;
        }
        switch (Filter.AttributeType)
        {
        case EShibMatchmakerHostConfigurationAttributeFilterType::String:
            Params.Add(
                FName(*Filter.AttributeName),
                FOnlineSessionSearchParam(Filter.AttributeValueString, Comparison));
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterType::Int64:
            Params.Add(FName(*Filter.AttributeName), FOnlineSessionSearchParam(Filter.AttributeValueInt64, Comparison));
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterType::Float:
            Params.Add(FName(*Filter.AttributeName), FOnlineSessionSearchParam(Filter.AttributeValueFloat, Comparison));
            break;
        case EShibMatchmakerHostConfigurationAttributeFilterType::Boolean:
            Params.Add(
                FName(*Filter.AttributeName),
                FOnlineSessionSearchParam(Filter.AttributeValueBoolean, Comparison));
            break;
        }
    }
    return Params;
}
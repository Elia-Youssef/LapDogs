#pragma once

#include "CoreMinimal.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibMatchmakingEOS.generated.h"

UENUM(BlueprintType)
enum class EShibMatchmakerHostConfigurationAttributeFilterType : uint8
{
	String,
	Int64,
	Float,
	Boolean,
};

UENUM(BlueprintType)
enum class EShibMatchmakerHostConfigurationAttributeFilterComparison : uint8
{
	Equal,
	NotEqual,
	LessThan,
	LessThanOrEqual,
	GreaterThan,
	GreaterThanOrEqual,
};

USTRUCT(BlueprintType)
struct SHIBMULTIPLAYER_API FShibMatchmakerHostConfigurationAttributeFilter
{
	GENERATED_BODY()

public:
	/**
	 * The attribute to filter on.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	FString AttributeName;

	/**
	 * The attribute type.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	EShibMatchmakerHostConfigurationAttributeFilterType AttributeType =
		EShibMatchmakerHostConfigurationAttributeFilterType::String;

	/**
	 * The attribute value, if type is set to string.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	FString AttributeValueString;

	/**
	 * The attribute value, if type is set to int64.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	int64 AttributeValueInt64 = 0;

	/**
	 * The attribute value, if type is set to float.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	float AttributeValueFloat = 0.0f;

	/**
	 * The attribute value, if type is set to boolean.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	bool AttributeValueBoolean = false;

	/**
	 * The comparison type to use.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	EShibMatchmakerHostConfigurationAttributeFilterComparison Comparison =
		EShibMatchmakerHostConfigurationAttributeFilterComparison::Equal;
};

USTRUCT(BlueprintType)
struct SHIBMULTIPLAYER_API FShibMatchmakerHostConfiguration
{
	GENERATED_BODY()
	
public:
    FShibMatchmakerHostConfiguration()
        : QueueName()
        , TeamCapacities()
        , SkillStatPrefix()
        , Map()
        , SessionFilters(){};

    /**
     * The name of the matchmaking queue to queue into.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString QueueName = TEXT("Default");
	
	/**
	 * A string like '3v3v3' (3 teams of 3) or '4x20' (20 teams of 4).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	FString TeamCapacities;

    /**
     * If true, finish matchmaking with a partially filled match if no further candidates could be found within the
     * specified timeout.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bAllowPartiallyFilledMatches = true;

    /**
     * If partial matches is enabled, this is the timeout in seconds after which, if we still can't find any other
     * candidates to match with, a partially filled match will be returned. Note that this is *not* a global timeout for
     * the entire matchmaking process. It is just the time since the last player was added to the match.
     *
     * The matchmaker has a minimum of 15 seconds for this value; if you set this to any lower than 15 seconds, the
     * matchmaker will use 15 seconds instead.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "15"))
    int32 NoCandidatesTimeout = 60;

    /**
     * The time in seconds to add to the timeout for each currently empty slot in the match. If this is non-zero,
     * matchmaking will wait longer to return a partial match, the more empty the match currently is.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "0"))
    int32 NoCandidatesTimeoutPerEmptySlot = 5;

    /**
     * If true, matchmaking will prioritize having balanced teams throughout the matchmaking process, rather than
     * prioritizing having completely full teams. Enabling this option means a partially filled match is more likely to
     * have a balanced number of players, at the cost of having a higher chance that a match will be partially filled if
     * there are not a lot of solo players entering the matchmaking queue.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches"))
    bool bPrioritizeBalance = false;

    /**
     * If set to something other than empty string, enables skill-based matchmaking. The <prefix>_mu and <prefix>_sigma
     * stats will be looked up in the online stats interface to perform skill-based matchmaking, so you must ensure
     * you've created the appropriate stats in the backend.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString SkillStatPrefix;

    /**
     * If true, the host will search for a dedicated server to play on once the match is ready, instead of hosting the
     * game themselves on a listen server.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bUseDedicatedServers = false;

    /**
     * The map to start when matchmaking is complete (listen servers only).
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Listen Servers)",
        meta = (EditCondition = "!bUseDedicatedServers"))
    TSoftObjectPtr<UWorld> Map;

    /**
     * The filters to apply when searching for dedicated servers.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    TArray<FShibMatchmakerHostConfigurationAttributeFilter> SessionFilters;
	
    /**
     * The port that the dedicated server matchmaking beacon listens on.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    FName BeaconPort = FName(TEXT("9990"));
};

USTRUCT(BlueprintType)
struct SHIBMULTIPLAYER_API FShibMatchmakerProgressInfo
{
	GENERATED_BODY()

public:
	FShibMatchmakerProgressInfo()
		: StepName()
		, CurrentStatus()
		, CurrentDetail()
		, CurrentProgress()
		, EstimatedTimeOfCompletion(){};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
	FName StepName;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
	FText CurrentStatus;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
	FText CurrentDetail;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
	float CurrentProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
	FDateTime EstimatedTimeOfCompletion = FDateTime();
};

// We just make blueprint visible events so we can pass the
// information up to our widgets
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnGameMatchmakingProgress, FName, StepName, FText, Status, FText, Detail, float, Progress, FDateTime, ETA);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameMatchmakingComplete, const FString&, TeamResults, const FShibMatchmakerHostConfiguration&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingError);

/**
 * 
 */
UCLASS()
class SHIBMULTIPLAYER_API UShibMatchmakingEOS : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	public:

	explicit UShibMatchmakingEOS();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingProgress OnProgress;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingComplete OnComplete;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingCancelled OnCancelled;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingError OnError;

	UFUNCTION(BlueprintCallable)
	void StartMatchmaking(FShibMatchmakerHostConfiguration MatchMakingHostConfig);

	UFUNCTION(BlueprintCallable)
	void CancelMatchmaking();
	
	/**
	* Returns whether local user is undergoing matchmaking. This is a utility function so you don't need to track
	* this via OnProgress/OnCancelled/OnError events if you just want to know if matchmaking is occurring.
	*/
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	void IsMatchmaking(bool &bIsMatchmaking);

	/**
	* Returns the latest matchmaking status reported by the OnProgress event. If you'd prefer to get the matchmaking
	* status directly instead of listening for OnProgress, you can use this function.
	*
	* If matchmaking isn't running, StepName will be NAME_None and the other values are undefined.
	*/
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	void GetMatchmakingStatus(FShibMatchmakerProgressInfo &ProgressInfo) const;

	/**
	* The matchmaker host configuration. This value is only used by the party leader.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Matchmaking")
	FShibMatchmakerHostConfiguration CachedHostConfig;

private:
	FMatchmakingEngineRequestHandle Handle;

	bool bIsMatchmaking;
	FShibMatchmakerProgressInfo LastMatchmakingProgressData;

	void OnHandleProgress(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineProgressInfo ProgressInfo);
	void OnHandleResultsReady(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response);
	void OnHandleComplete(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response);
	void OnHandleCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request);
	void OnHandleError(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		const FOnlineError &Error);
	FSearchParams OnHandleGetDedicatedServerSearchParams(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response,
		int32 SearchIteration);
};

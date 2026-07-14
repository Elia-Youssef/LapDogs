// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "TournamentsApisTypes.generated.h"

USTRUCT()
struct FPlayerAddressRequest
{
	GENERATED_BODY()

	FString player = FString();
};

USTRUCT()
struct FTournamentAddressRequest
{
	GENERATED_BODY()

	FString address = FString();
};

USTRUCT()
struct FTournamentPlayerRequest
{
	GENERATED_BODY()

	FString address = FString();
	FString player = FString();
};

USTRUCT()
struct FTournamentPlayerScoreRequest
{
	GENERATED_BODY()

	FString address = FString();
	FString player = FString();
	int32 score = 0;
};


USTRUCT(BlueprintType)
struct FTournamentDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TournamentId = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TournamentName = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EntryCurrency = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameName = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinimumParticipants = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaximumParticipants = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime = FDateTime{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime = FDateTime{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RegistrationStartTime = FDateTime{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RegistrationEndTime = FDateTime{};
};

USTRUCT(BlueprintType)
struct FTournamentPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WalletAddress = FString();
};

USTRUCT(BlueprintType)
struct FTournamentRewards
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Currency = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartRank = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndRank = -1;
};

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UTournamentsApisTypes : public UObject
{
	GENERATED_BODY()
};

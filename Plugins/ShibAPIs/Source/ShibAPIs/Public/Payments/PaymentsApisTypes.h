// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "PaymentsApisTypes.generated.h"

UENUM(BlueprintType)
enum class EShibCurrencies : uint8
{
	SC_None = 0 UMETA(DisplayName = "NONE"),
	SC_ShibCredit = 1 UMETA(DisplayName = "ShibCredit"),
	SC_GrandPrixToken = 2 UMETA(DisplayName = "GrandPrixToken"),
	SC_Ticket = 3 UMETA(DisplayName = "Ticket")
};

UENUM(BlueprintType)
enum class ETransactionStatus : uint8
{
	TS_None = 0 UMETA(DisplayName = "NONE"),
	TS_Pending = 1 UMETA(DisplayName = "Pending"),
	TS_Failed = 2 UMETA(DisplayName = "Failed"),
	TS_Successful = 3 UMETA(DisplayName = "Successful")
};

USTRUCT(BlueprintType)
struct FBalance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibCurrencies Currency = EShibCurrencies::SC_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount = 0.f;
};

USTRUCT(BlueprintType)
struct FTransactionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShibCurrencies Currency = EShibCurrencies::SC_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionId = FString();
};

USTRUCT(BlueprintType)
struct FCryptoTransactionDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Url = FString();
};

USTRUCT(BlueprintType)
struct FTransactionStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETransactionStatus Status = ETransactionStatus::TS_None;
};

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UPaymentsApisTypes : public UObject
{
	GENERATED_BODY()

};

// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsTypes.generated.h"

/**
 * ============ USER INFO ============
 */

// Shib User
USTRUCT(BlueprintType)
struct FShibUser
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Id = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString UserName = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Email = FString();
};

// Shib User's NFTs
USTRUCT(BlueprintType)
struct FShibWallet
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Id = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString WalletAddress = FString();
};

// Shib User's NFTs
USTRUCT(BlueprintType)
struct FShibNFTs
{
	GENERATED_BODY()
};

// Get Shib User Info JSON Response
USTRUCT(BlueprintType)
struct FGetUserInfoResponse
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FShibUser User = FShibUser();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FShibWallet Wallet = FShibWallet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FShibNFTs> NFTs = TArray<FShibNFTs>();
};

USTRUCT(BlueprintType)
struct FUserRegion
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString ip = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString city = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString region = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString FullJsonResponse = FString();
};

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UShibAPIsTypes : public UObject
{
	GENERATED_BODY()

public:
	static TSharedPtr<FJsonObject> ParseJsonAsObject(const FString& JsonData);
	static TArray<TSharedPtr<FJsonValue>> ParseJsonAsArray(const FString& JsonData);
};

// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "MetaverseApisTypes.generated.h"

USTRUCT(BlueprintType)
struct FServerPlotInfo
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlotId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Owner = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 X = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Y = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString District = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString PlotData = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Tier = FString();
};

USTRUCT(BlueprintType)
struct FPlotDataRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString PlotData = FString();
};

USTRUCT(BlueprintType)
struct FMediaStream
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 id;

	UPROPERTY(BlueprintReadWrite)
	bool Stream = false;

	UPROPERTY(BlueprintReadWrite)
	bool SyncStream = false;

	UPROPERTY(BlueprintReadWrite)
	FString URL = FString();
};

USTRUCT(BlueprintType)
struct FSlideshowImage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Url = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Time = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2DDynamic* Texture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MediaType = 0;
};

UENUM(BlueprintType)
enum class EMediaType : uint8
{
	MT_None = 0 UMETA(DisplayName = "NONE"),
	MT_Video = 1 UMETA(DisplayName = "Video"),
	MT_Image = 2 UMETA(DisplayName = "Image"),
	MT_Text = 3 UMETA(DisplayName = "Text")
};

USTRUCT(BlueprintType)
struct FMedia {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StreamerId = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMediaType MediaType = EMediaType::MT_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString URL = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlideshowTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text = FString();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TextAr = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Interactable = false;
	
	bool operator==(const FMedia& Other) const
	{
		return Id == Other.Id;
	}
};

USTRUCT(BlueprintType)
struct FShibLog
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString eventCode = FString();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString eventDescription = FString();
};

USTRUCT(BlueprintType)
struct FMvUserDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UserId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Username = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WalletAddress = FString();
};

USTRUCT(BlueprintType)
struct FMvPlayerDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShibCredit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 0;
};

USTRUCT(BlueprintType)
struct FMvAvatarDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AvatarData = FString();
};

USTRUCT(BlueprintType)
struct FShibMvPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMvUserDetails User = FMvUserDetails();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMvPlayerDetails Player = FMvPlayerDetails();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMvAvatarDetails Avatar = FMvAvatarDetails();
};

USTRUCT(BlueprintType)
struct FFishingSpotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rarity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Enabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpotId = FString();
};

USTRUCT(BlueprintType)
struct FShibSavedItem
{
	GENERATED_BODY()

	UPROPERTY(Category = "")
	int32 ItemId = -1;
	
	UPROPERTY(Category = "")
	FName Name = "";

	UPROPERTY(BlueprintReadOnly, Category = "")
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "")
	float Price = 0;
};

USTRUCT(BlueprintType)
struct FSaveInventoryRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "")
	int32 UserId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "")
	TArray<FShibSavedItem> Items = {};
};

USTRUCT(BlueprintType)
struct FShibMediaPlaylist
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = FString();
};

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UMetaverseApisTypes : public UObject
{
	GENERATED_BODY()

};

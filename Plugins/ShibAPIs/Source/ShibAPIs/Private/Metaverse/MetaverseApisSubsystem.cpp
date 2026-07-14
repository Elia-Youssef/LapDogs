// Copyright Shiba Inu Games LLC.

#include "Metaverse/MetaverseApisSubsystem.h"

#include "ShibAPIsTypes.h"
#include "ShibAPIsUtils.h"
#include "Kismet/KismetMathLibrary.h"


void UMetaverseApisSubsystem::GetMvPlayer()
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetMvPlayer);
	HttpRequest("Get", GetMvPlayerEndpoint, FString(), Callback);
}

void UMetaverseApisSubsystem::OnGetMvPlayer(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetMvPlayerDelegate.Broadcast(FShibMvPlayer(), false);
		return;
	}

	bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<FShibMvPlayer>(ResponseAsString, &MvPlayer);
	OnGetMvPlayerDelegate.Broadcast(MvPlayer, bParsed);
}

void UMetaverseApisSubsystem::GetMvPlayerById(int32 UserId, const TFunction<void(FShibMvPlayer)>& OnGet)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([OnGet](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			OnGet(FShibMvPlayer());
			return;
		}

		FShibMvPlayer Player;
		bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<FShibMvPlayer>(ResponseAsString, &Player);
		OnGet(Player);
	});

	HttpRequest("Get", FString::Printf(TEXT("%s?UserId=%d"), *GetMvPlayerByIdEndpoint, UserId), FString(),
	            Callback);
}

void UMetaverseApisSubsystem::UpdateMvPlayerById(int32 UserId, const FMvPlayerDetails& PlayerDetails)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([this](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		OnUpdateMvPlayerByIdDelegate.Broadcast(bSuccessful);
	});

	HttpRequest("Post", FString::Printf(TEXT("%s?UserId=%d"), *UpdateMvPlayerByIdEndpoint, UserId),
	            UShibAPIsUtils::StructToJson(PlayerDetails), Callback);
}

void UMetaverseApisSubsystem::Log(const FShibLog& Log)
{
	HttpRequest("Post", MvLoggingEndpoint, UShibAPIsUtils::StructToJson(Log));
}

void UMetaverseApisSubsystem::PostAvatarInfo(const FString& Content)
{
	// TODO: use the token only for this api
	if (UserInfo.User.Id == -1)
	{
		OnPostAvatarInfoDelegate.Broadcast(false);
		return;
	}

	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnPostAvatarInfo);
	HttpRequest(
		"Post", FString::Printf(TEXT("%s?UserId=%d"), *PostAvatarInfoEndpoint, UserInfo.User.Id),
		UShibAPIsUtils::StructToJson(FMvAvatarDetails{Content}),
		Callback);
}

void UMetaverseApisSubsystem::OnPostAvatarInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	OnPostAvatarInfoDelegate.Broadcast(bSuccessful);
}

void UMetaverseApisSubsystem::PostPlotInfo(const int32& PlotId, const FString& Content)
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnPostPlotInfo);
	HttpRequest("Post",
	            FString::Printf(TEXT("%s?PlotId=%d"), *PostPlotInfoEndpoint, PlotId),
	            UShibAPIsUtils::StructToJson(FPlotDataRequest{Content}), Callback);
}

void UMetaverseApisSubsystem::OnPostPlotInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	OnPostPlotInfoDelegate.Broadcast(bSuccessful);
}

void UMetaverseApisSubsystem::GetPlotInfo(int32 X, int32 Y)
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetPlotInfo);
	HttpRequest("Get", FString::Printf(TEXT("%s?coordx=%d&coordy=%d"), *GetPlotInfoEndpoint, X, Y), FString(),
	            Callback);
}

void UMetaverseApisSubsystem::OnGetPlotInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetPlotInfoDelegate.Broadcast(FServerPlotInfo(), false);
		return;
	}

	FServerPlotInfo GetPlotInfoResponse;
	bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct<FServerPlotInfo>(
		ResponseAsString, &GetPlotInfoResponse);
	OnGetPlotInfoDelegate.Broadcast(GetPlotInfoResponse, bParsed);
}

void UMetaverseApisSubsystem::GetAllPlotsInfo(FVector2D From, FVector2D To)
{
	FString QueryParams;
	// from < to
	QueryParams.Append(FString::Printf(
		TEXT("fromx=%d&tox=%d"),
		static_cast<int32>(FMath::Min(From.X, To.X)),
		static_cast<int32>(FMath::Max(From.X, To.X))));
	QueryParams.Append(FString::Printf(
		TEXT("&fromy=%d&toy=%d"),
		static_cast<int32>(FMath::Min(From.Y, To.Y)),
		static_cast<int32>(FMath::Max(From.Y, To.Y))));

	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetAllPlotsInfo);
	HttpRequest("Get", FString::Printf(TEXT("%s?%s"), *GetAllPlotsInfoEndpoint, *QueryParams), FString(),
	            Callback);
}

void UMetaverseApisSubsystem::OnGetAllPlotsInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetAllPlotsInfoDelegate.Broadcast(TArray<FServerPlotInfo>(), false, true);
		return;
	}

	TArray<FServerPlotInfo> GetAllPlotsInfoResponse;
	bool bParsed = FJsonObjectConverter::JsonArrayStringToUStruct<FServerPlotInfo>(
		ResponseAsString, &GetAllPlotsInfoResponse);
	OnGetAllPlotsInfoDelegate.Broadcast(GetAllPlotsInfoResponse, bParsed, true);
}

void UMetaverseApisSubsystem::GetMyPlots()
{
	if (MyPlotsInfoLastUpdateTime != FDateTime()) // When it's not the first time we update the plots info
	{
		const FTimespan Diff = UKismetMathLibrary::Subtract_DateTimeDateTime(
			FDateTime::Now(), MyPlotsInfoLastUpdateTime);

		// If the plots info we have is not outdated, we simply send back the info we already have
		if (Diff.GetTotalMinutes() < MyPlotsInfoTimeLifespan)
		{
			OnGetMyPlotsDelegate.Broadcast(MyPlotsInfo, true, false);
			return;
		}
	}

	// Record the time when we've updated MyPlotsInfo
	MyPlotsInfoLastUpdateTime = FDateTime::Now();

	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetMyPlots);
	HttpRequest("Get", FString::Printf(TEXT("%s"), *GetMyPlotsEndpoint), FString(), Callback);
}

void UMetaverseApisSubsystem::OnGetMyPlots(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetMyPlotsDelegate.Broadcast(TArray<FServerPlotInfo>(), false, true);
		return;
	}

	MyPlotsInfo.Empty();
	bool bParsed = FJsonObjectConverter::JsonArrayStringToUStruct<FServerPlotInfo>(ResponseAsString, &MyPlotsInfo);
	OnGetMyPlotsDelegate.Broadcast(MyPlotsInfo, bParsed, true);
}

void UMetaverseApisSubsystem::GetMedia(const TArray<FString>& MediaIds)
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnGetMedia);
	HttpRequest(
		"Post",
		FString::Printf(
			TEXT("%s"),
			*GetMediaEndpoint),
		UShibAPIsUtils::StringArrayToJson(MediaIds),
		Callback);
}

void UMetaverseApisSubsystem::OnGetMedia(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnGetMediaDelegate.Broadcast(TArray<FMedia>(), false);
		return;
	}

	TArray<TSharedPtr<FJsonValue>> JsonData = UShibAPIsTypes::ParseJsonAsArray(ResponseAsString);
	if (!JsonData.Num())
	{
		OnGetMediaDelegate.Broadcast(TArray<FMedia>(), true);
		return;
	}

	TArray<FMedia> GetMediaResponse;
	for (auto Value : JsonData)
	{
		FMedia ValueAsStruct;

		FString MediaTypeString = Value->AsObject()->GetStringField(TEXT("MediaType"));
		Value->AsObject()->RemoveField(TEXT("MediaType"));

		FJsonObjectConverter::JsonObjectToUStruct<FMedia>(Value->AsObject().ToSharedRef(), &ValueAsStruct);
		ValueAsStruct.MediaType = MediaTypeString == "image"
			                          ? EMediaType::MT_Image
			                          : MediaTypeString == "video"
			                          ? EMediaType::MT_Video
			                          : EMediaType::MT_None;

		GetMediaResponse.Add(ValueAsStruct);
	}

	OnGetMediaDelegate.Broadcast(GetMediaResponse, true);
}

void UMetaverseApisSubsystem::IsAlwahaEnabled()
{
	FHttpRequestCallback Callback;
	Callback.BindUObject(this, &ThisClass::OnIsAlwahaEnabled);
	HttpRequest("Get", IsAlWahaEnabledEndpoint, FString(), Callback);
}

void UMetaverseApisSubsystem::OnIsAlwahaEnabled(bool bSuccessful, int32 Status, const FString& ResponseAsString)
{
	if (!bSuccessful)
	{
		OnIsAlwahaEnabledDelegate.Broadcast(false, false);
		return;
	}

	TSharedPtr<FJsonObject> JsonData = UShibAPIsTypes::ParseJsonAsObject(ResponseAsString);
	if (!JsonData)
	{
		OnIsAlwahaEnabledDelegate.Broadcast(false, bSuccessful);
		return;
	}

	bIsAlwahaEnabled = JsonData->GetBoolField(TEXT("EnableAlWaha"));
	OnIsAlwahaEnabledDelegate.Broadcast(bIsAlwahaEnabled, bSuccessful);
}

void UMetaverseApisSubsystem::GetSpotsData(const TArray<FString>& SpotsIds,
                                           const TFunction<void(TArray<FFishingSpotData>)>& Callback)
{
	if (!Callback) return;

	FHttpRequestCallback RequestCallback;
	RequestCallback.BindLambda([Callback](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		TArray<FFishingSpotData> SpotsData;
		FJsonObjectConverter::JsonArrayStringToUStruct<FFishingSpotData>(ResponseAsString, &SpotsData);
		Callback(SpotsData);
	});

	HttpRequest("Post", GetSpotsDataEndpoint, UShibAPIsUtils::StringArrayToJson(SpotsIds), RequestCallback);
}

void UMetaverseApisSubsystem::LoadInventory(int32 UserId,
                                            const TFunction<void(const TArray<FShibSavedItem>&)>& Callback)
{
	if (UserId <= 0) return;

	FHttpRequestCallback RequestCallback;
	RequestCallback.BindLambda([Callback](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (Callback)
		{
			TArray<FShibSavedItem> Items;
			FJsonObjectConverter::JsonArrayStringToUStruct<FShibSavedItem>(ResponseAsString, &Items);
			Callback(Items);
		}
	});

	HttpRequest("Get", FString::Printf(TEXT("%s?UserId=%d"), *LoadInventoryEndpoint, UserId), FString(),
	            RequestCallback);
}

void UMetaverseApisSubsystem::SaveInventory(const FSaveInventoryRequest& Inventory,
                                            const TFunction<void()>& Callback)
{
	if (Inventory.UserId <= 0) return;

	FHttpRequestCallback RequestCallback;
	RequestCallback.BindLambda([Callback](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (Callback)
		{
			Callback();
		}
	});

	HttpRequest("Post", SaveInventoryEndpoint, UShibAPIsUtils::StructToJson(Inventory), RequestCallback);
}

void UMetaverseApisSubsystem::GetAllPlaylists()
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([this](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			OnGetPlaylistsDelegate.Broadcast(TArray<FShibMediaPlaylist>(), true);
			return;
		}

		TArray<FShibMediaPlaylist> Playlists;
		bool bParsed = FJsonObjectConverter::JsonArrayStringToUStruct<FShibMediaPlaylist>(ResponseAsString, &Playlists);
		OnGetPlaylistsDelegate.Broadcast(Playlists, bParsed);
	});
	HttpRequest("Get", GetAllPlaylistsEndpoint, FString(), Callback);
}

void UMetaverseApisSubsystem::GetPlaylistMedia(int32 PlaylistId)
{
	FHttpRequestCallback Callback;
	Callback.BindLambda([this](bool bSuccessful, int32 Status, const FString& ResponseAsString)
	{
		if (!bSuccessful)
		{
			OnGetPlaylistMediaDelegate.Broadcast(TArray<FMedia>(), false);
			return;
		}

		TArray<TSharedPtr<FJsonValue>> JsonData = UShibAPIsTypes::ParseJsonAsArray(ResponseAsString);
		if (!JsonData.Num())
		{
			OnGetPlaylistMediaDelegate.Broadcast(TArray<FMedia>(), true);
			return;
		}

		TArray<FMedia> GetMediaResponse;
		for (auto Value : JsonData)
		{
			FMedia ValueAsStruct;

			FString MediaTypeString = Value->AsObject()->GetStringField(TEXT("MediaType"));
			Value->AsObject()->RemoveField(TEXT("MediaType"));

			FJsonObjectConverter::JsonObjectToUStruct<FMedia>(Value->AsObject().ToSharedRef(), &ValueAsStruct);
			ValueAsStruct.MediaType = MediaTypeString == "image"
				                          ? EMediaType::MT_Image
				                          : MediaTypeString == "video"
				                          ? EMediaType::MT_Video
				                          : EMediaType::MT_None;

			GetMediaResponse.Add(ValueAsStruct);
		}

		OnGetPlaylistMediaDelegate.Broadcast(GetMediaResponse, true);
	});
	
	HttpRequest("Get", FString::Printf(TEXT("%s?PlaylistId=%d"), *GetPlaylistMediaEndpoint, PlaylistId), FString(),
	            Callback);
}

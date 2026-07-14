// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "MetaverseApisTypes.h"
#include "ShibAPIsSubsystem.h"
#include "MetaverseApisSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetMvPlayer, FShibMvPlayer, Player, bool, bSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetPlotInfoDelegate, FServerPlotInfo, GetPlotInfoResponse, bool,
                                             bSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGetAllPlotsInfoDelegate, const TArray<FServerPlotInfo>&,
                                               GetAllPlotsInfoResponse, bool, bSuccessful, bool, bInfoUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetMedia, const TArray<FMedia>&, Media, bool, bSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIsAlwahaEnabled, bool, bEnabled, bool, bSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetPlaylists, const TArray<FShibMediaPlaylist>&, Playlists, bool, bSuccessful);

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UMetaverseApisSubsystem : public UShibAPIsSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FShibMvPlayer MvPlayer;

	UPROPERTY(BlueprintReadOnly)
	TArray<FServerPlotInfo> MyPlotsInfo;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAlwahaEnabled = false;

private:
	// The time in between two updates of MyPlotsInfo variable.
	// This is used to limit the amount of calls we do to the API.
	// This variable represents minutes (3.f = 3 minutes)
	float MyPlotsInfoTimeLifespan = 3.f;

	// The last time the MyPlotsInfo variable got updated
	FDateTime MyPlotsInfoLastUpdateTime = FDateTime();

public:
	/**
	 * GET MV PLAYER
	 */
	FString GetMvPlayerEndpoint = FString("/api/MV/GETMVPLAYER");
	UFUNCTION(BlueprintCallable, Category = "Requests|Player")
	void GetMvPlayer();
	UFUNCTION()
	void OnGetMvPlayer(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Player|Delegates")
	FOnGetMvPlayer OnGetMvPlayerDelegate;

	/**
	 * GET MV PLAYER BY ID
	 */
	FString GetMvPlayerByIdEndpoint = FString("/api/MV/GETMVPLAYERBYID");
	void GetMvPlayerById(int32 UserId, const TFunction<void(FShibMvPlayer)>& OnGet);

	/**
	 * UPDATE MV PLAYER BY ID
	 */
	FString UpdateMvPlayerByIdEndpoint = FString("/api/MV/UPDATEMVPLAYERS");
	UFUNCTION(BlueprintCallable, Category = "Requests|Player")
	void UpdateMvPlayerById(int32 UserId, const FMvPlayerDetails& PlayerDetails);
	FOnResponse OnUpdateMvPlayerByIdDelegate;

public:
	/**
	 *  MV LOGGING
	 */
	FString MvLoggingEndpoint = FString("/api/MV/INSERTLOG");
	UFUNCTION(BlueprintCallable, Category = "Requests|Logs")
	void Log(const FShibLog& Log);

	/**
	 * POST AVATAR DATA
	 */
	FString PostAvatarInfoEndpoint = FString("/api/MV/INSERTUPDATEAVATAR");
	UFUNCTION(BlueprintCallable, Category = "Requests|Avatar")
	void PostAvatarInfo(const FString& Content);
	UFUNCTION()
	void OnPostAvatarInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Avatar|Delegates")
	FOnResponse OnPostAvatarInfoDelegate;

	/**
	 * POST PLOT DATA
	 */
	FString PostPlotInfoEndpoint = FString("/api/MV/INSERTUPDATEPLOT");
	UFUNCTION(BlueprintCallable, Category = "Requests|Plots")
	void PostPlotInfo(const int32& PlotId, const FString& Content);
	UFUNCTION()
	void OnPostPlotInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Plots|Delegates")
	FOnResponse OnPostPlotInfoDelegate;

	/**
	 * GET PLOT DATA
	 */
	FString GetPlotInfoEndpoint = FString("/api/MV/GETPLOT");
	UFUNCTION(BlueprintCallable, Category = "Requests|Plots")
	void GetPlotInfo(int32 X, int32 Y);
	UFUNCTION()
	void OnGetPlotInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Plots|Delegates")
	FOnGetPlotInfoDelegate OnGetPlotInfoDelegate;

	/**
	 * GET ALL PLOTS
	 */
	FString GetAllPlotsInfoEndpoint = FString("/api/MV/GETALLPLOTS");
	UFUNCTION(BlueprintCallable, Category = "Requests|Plots")
	void GetAllPlotsInfo(FVector2D From, FVector2D To);
	UFUNCTION()
	void OnGetAllPlotsInfo(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Plots|Delegates")
	FOnGetAllPlotsInfoDelegate OnGetAllPlotsInfoDelegate;

	/**
	 * GET MY PLOTS
	 */
	FString GetMyPlotsEndpoint = FString("/api/MV/GETUSERPLOTS");
	UFUNCTION(BlueprintCallable, Category = "Requests|Plots")
	void GetMyPlots();
	UFUNCTION()
	void OnGetMyPlots(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Plots|Delegates")
	FOnGetAllPlotsInfoDelegate OnGetMyPlotsDelegate;

	/**
	 * GET MEDIA
	 */
	FString GetMediaEndpoint = FString("/api/MV/GETMEDIA");
	UFUNCTION(BlueprintCallable, Category = "Requests|Stream")
	void GetMedia(const TArray<FString>& MediaIds);
	UFUNCTION()
	void OnGetMedia(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Stream|Delegates")
	FOnGetMedia OnGetMediaDelegate;

	/**
	 * IS AL WAHA ENABLED
	 */
	FString IsAlWahaEnabledEndpoint = FString("/api/MV/ENABLEALWAHA");
	UFUNCTION(BlueprintCallable, Category = "Requests|Config")
	void IsAlwahaEnabled();
	UFUNCTION()
	void OnIsAlwahaEnabled(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Config|Delegates")
	FOnIsAlwahaEnabled OnIsAlwahaEnabledDelegate;

	/**
	 * Get Fishing Spots details
	 */
	FString GetSpotsDataEndpoint = FString("/api/MV/GETSPOT");
	void GetSpotsData(const TArray<FString>& SpotsIds, const TFunction<void(TArray<FFishingSpotData>)>& Callback);
	
	/**
	 * Save MV inventory
	 */
	FString LoadInventoryEndpoint = FString("/api/MV/GETMVINVENTORY");
	void LoadInventory(int32 UserId, const TFunction<void(const TArray<FShibSavedItem>&)>& Callback);
	
	/**
	 * Load MV inventory
	 */
	FString SaveInventoryEndpoint = FString("/api/MV/INSERTUPDATEINVENTORY");
	void SaveInventory(const FSaveInventoryRequest& Inventory, const TFunction<void()>& Callback = nullptr);

	/**
	 * Get All Media Playlists
	 */
	FString GetAllPlaylistsEndpoint = FString("/api/MV/GETPLAYLIST");
	UFUNCTION(BlueprintCallable, Category = "Requests|Stream")
	void GetAllPlaylists();
	UPROPERTY(BlueprintAssignable, Category = "Requests|Stream|Delegates")
	FOnGetPlaylists OnGetPlaylistsDelegate;

	/**
	 * GET PLAYLIST MEDIA
	 */
	FString GetPlaylistMediaEndpoint = FString("/api/MV/GETMEDIABYPLAYLIST");
	UFUNCTION(BlueprintCallable, Category = "Requests|Stream")
	void GetPlaylistMedia(int32 PlaylistId);
	UPROPERTY(BlueprintAssignable, Category = "Requests|Stream|Delegates")
	FOnGetMedia OnGetPlaylistMediaDelegate;
};

// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibChunkingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMountCompleteDelegate, bool, Succeeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadProgressDelegate,float,Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMountProgressDelegate,float,Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateFailedDelegate);

/**
 * 
 */
UCLASS()
class SHIBCHUNKINGSYSTEM_API UShibChunkingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
public:
	
	UFUNCTION(BlueprintPure, Category = "Patching|Stats")
	void GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

	void ExtractChunkIDsFromManifest();
	
	void WaitForGameWorld();

	UPROPERTY(BlueprintAssignable, Category="Patching")
	FPatchCompleteDelegate OnPatchComplete;

	UPROPERTY(BlueprintAssignable, Category="Patching")
	FMountCompleteDelegate OnMountCompleteDelegate;

	UPROPERTY(BlueprintAssignable, Category="Patching")
	FUpdateFailedDelegate UpdateFailedDelegate;

	UFUNCTION(BlueprintCallable, Category = "Patching")
	bool PatchGame();

	UPROPERTY(BlueprintReadWrite, Category = "Patching|Stats")
	bool IsPatchingInProgress;

	UPROPERTY(BlueprintReadWrite, Category = "Patching|Stats")
	int TryPatchCount;

	UPROPERTY(BlueprintReadWrite, Category = "Patching|TimerHandles")
	FTimerHandle PatchLoopTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Patching|TimerHandles")
	FTimerHandle UpdateTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Patching|TimerHandles")
	FTimerHandle LevelOpenTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Patching|TimerHandles")
	FTimerHandle GameWorldTimerHandle;

	UPROPERTY(BlueprintAssignable, Category="Patching");
	FDownloadProgressDelegate DownloadProgressDelegate;

	UPROPERTY(BlueprintAssignable, Category="Patching");
	FMountProgressDelegate MountProgressDelegate;
	
	void PatchLoop();

	void UpdateProgress();

	void LevelOpen();
	
	UFUNCTION()
	void LevelOpenFunction();
	
	UFUNCTION(BlueprintCallable)
	void CheckForUpdates();
	
protected:

	bool bIsDownloadManifestUpToDate;
	
	void OnManifestUpdateComplete(bool bSuccess);

	UPROPERTY(EditDefaultsOnly, Category="Patching")
	TArray<int32> ChunkDownloadList;

	// Called when the chunk download process finishes
	void OnDownloadComplete(bool bSuccess);

	// Called whenever ChunkDownloader's loading mode is finished
	void OnLoadingModeComplete(bool bSuccess);
 
	// Called when ChunkDownloader finishes mounting chunks
	void OnMountComplete(bool bSuccess);
};


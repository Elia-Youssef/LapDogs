// Copyright Shiba Inu Games LLC.


#include "ShibChunkingSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"

void UShibChunkingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UShibChunkingSubsystem::Deinitialize()
{
	Super::Deinitialize();
	FChunkDownloader::Shutdown();
}

void UShibChunkingSubsystem::CheckForUpdates()
{
	const FString DeploymentName = "ShibCDN";
	
	// getting latest content build id
	FString ConfigSectionName = FString::Printf(TEXT("/Script/Plugins.ChunkDownloader %s"), *DeploymentName);
	TArray<FString> CdnBaseUrls;
	GConfig->GetArray(*ConfigSectionName, TEXT("CdnBaseUrls"), CdnBaseUrls, GGameIni);

	FString CdnBaseUrl=CdnBaseUrls[0];
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindLambda([CdnBaseUrl,DeploymentName,this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful && Response.IsValid())
        {
	        int32 ResponseCode = Response->GetResponseCode();

			if (ResponseCode == 200)  // HTTP OK
			{
				FString ResponseContent = Response->GetContentAsString();
				TArray<FString> Directories;

				// Regex to match directory names in the HTML
				FRegexPattern Pattern(TEXT("<A HREF=\"[^\"]+\">([^<]+)</A>"));
				FRegexMatcher Matcher(Pattern, ResponseContent);

				while (Matcher.FindNext())
				{
					FString DirectoryName = Matcher.GetCaptureGroup(1);
	                
					// Skip parent directory link
					if (DirectoryName != TEXT("[To Parent Directory]"))
					{
						Directories.Add(DirectoryName);
					}
				}

				// Sort the directories alphanumerically
				Directories.Sort([](const FString& A, const FString& B) {
					return A < B;
				});

				// The latest directory is the last one
				FString LatestDirectory = Directories.Last();

				UE_LOG(LogTemp, Log, TEXT("Latest Directory: %s"), *LatestDirectory);

				const FString ContentBuildId = LatestDirectory;
        		
				// initialize the chunk downloader with chosen platform
				TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
				Downloader->Initialize("Windows", 8);
			 
				// load the cached build ID
				Downloader->LoadCachedBuild(DeploymentName);
        		
				// update the build manifest file
				TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess)
				{
					bIsDownloadManifestUpToDate = true;
					//Populate the ChunkDownloadList Array
					ExtractChunkIDsFromManifest();
				};
				Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
				
				if (!IsValid(GetWorld()))
				{
					// Sometimes game world will not be valid and cause a crash, so we need to wait for it to be valid
					UE_LOG(LogTemp, Warning, TEXT("World Not valid Yet"));
					GetWorld()->GetTimerManager().SetTimer(GameWorldTimerHandle, this, &UShibChunkingSubsystem::WaitForGameWorld, 1.0f, true);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("World is Valid"));
	        		
					GetWorld()->GetTimerManager().SetTimer(PatchLoopTimerHandle, this, &UShibChunkingSubsystem::PatchLoop, 0.2f, true);
	                
					GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UShibChunkingSubsystem::UpdateProgress, 0.1f, true);
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to fetch directories from: %s"), *CdnBaseUrl);
				UpdateFailedDelegate.Broadcast();
			}
        }
        else
        {
        	GEngine->AddOnScreenDebugMessage(-1, 0.8, FColor::Red, FString::Printf(TEXT("Server is Down --- UPDATE FAILED ")));
        	UpdateFailedDelegate.Broadcast();
        }
    });

    // Set up the request
    Request->SetURL(CdnBaseUrl);
    Request->SetVerb(TEXT("GET"));
    Request->ProcessRequest();
	
}

void UShibChunkingSubsystem::ExtractChunkIDsFromManifest()
{
	ChunkDownloadList.Empty();
	UE_LOG(LogTemp, Log, TEXT("Emptied Chunk Download List"));
	
	// Define the file path for CachedBuildManifest inside the latest directory
	FString CacheFolder = FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/");
	const FString ManifestFilePath = FPaths::Combine(CacheFolder, TEXT("CachedBuildManifest.txt"));

	// Read the contents of the CachedBuildManifest file
	FString ManifestContent;
	if (!FFileHelper::LoadFileToString(ManifestContent, *ManifestFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read the CachedBuildManifest.txt file at %s"), *ManifestFilePath);
		return;
	}

	// Split the file content into lines
	TArray<FString> Lines;
	ManifestContent.ParseIntoArrayLines(Lines);

	// Iterate through the lines starting from the third line (index 2) to extract chunk IDs
	for (int32 i = 2; i < Lines.Num(); i++)
	{
		// Split each line by tab to get the data fields
		TArray<FString> Fields;
		Lines[i].ParseIntoArray(Fields, TEXT("\t"), true);

		// Check if the line has enough fields (expecting at least 4: filename, filesize, fileversion, chunkID)
		if (Fields.Num() >= 4)
		{
			int32 ChunkID = FCString::Atoi(*Fields[3]); //Extracting the Chunk ID (4th field)
			UE_LOG(LogTemp, Log, TEXT(">Extracted Chunk ID: %d"), ChunkID);
			ChunkDownloadList.Add(ChunkID);
		}
	}

	// Log the extracted Chunk IDs
	for (const int32& ID : ChunkDownloadList)
	{
		UE_LOG(LogTemp, Log, TEXT("Extracted Chunk ID: %d"), ID);
	}
}

void UShibChunkingSubsystem::WaitForGameWorld()
{
	if (IsValid(GetWorld()))
	{
		// Clearing the timer, so it does not keep looping after game world is valid
		GetWorld()->GetTimerManager().ClearTimer(GameWorldTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(PatchLoopTimerHandle, this, &UShibChunkingSubsystem::PatchLoop, 0.2f, true);
                
		GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UShibChunkingSubsystem::UpdateProgress, 0.01f, true);
	}
}

void UShibChunkingSubsystem::LevelOpenFunction()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("Shib_TestLevel"));
}

void UShibChunkingSubsystem::PatchLoop()
{
	IsPatchingInProgress=PatchGame();
	if (!IsPatchingInProgress)
	{
		TryPatchCount++;
		if (TryPatchCount<=3)
		{
			UGameplayStatics::OpenLevel(GetWorld(), FName("Shib_MultiplayerMainMenu"));
			UE_LOG(LogTemp, Log, TEXT("Failed To Update"));
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(PatchLoopTimerHandle);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(PatchLoopTimerHandle);
	}
}

void UShibChunkingSubsystem::UpdateProgress()
{
	if (IsPatchingInProgress)
	{
		int32 BytesDownloaded;
		int32 TotalBytesToDownload;
		float DownloadPercent;
		int32 ChunksMounted;
		int32 TotalChunksToMount;
		float MountPercent;
		GetLoadingProgress(BytesDownloaded, TotalBytesToDownload, DownloadPercent, ChunksMounted, TotalChunksToMount, MountPercent);

		DownloadProgressDelegate.Broadcast(DownloadPercent);
		MountProgressDelegate.Broadcast(MountPercent);
		
		if (ChunksMounted==TotalChunksToMount)
		{
			//UE_LOG(LogTemp, Error, TEXT("ChunksMounted==TotalChunksToMount"));
		}
	}
}

void UShibChunkingSubsystem::OnManifestUpdateComplete(bool bSuccess)
{
	bIsDownloadManifestUpToDate = bSuccess;
}

void UShibChunkingSubsystem::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
	//Get a reference to ChunkDownloader
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
	
	//Get the loading stats struct
	FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();
	
	//Get the bytes downloaded and bytes to download
	BytesDownloaded = LoadingStats.BytesDownloaded;

	TotalBytesToDownload = LoadingStats.TotalBytesToDownload;
	
	//Get the number of chunks mounted and chunks to download
	ChunksMounted = LoadingStats.ChunksMounted;

	TotalChunksToMount = LoadingStats.TotalChunksToMount;
	
	
	//Calculate the download and mount percent using the above stats
	DownloadPercent = ((float)BytesDownloaded / (float)TotalBytesToDownload) * 100.0f;
	MountPercent = ((float)ChunksMounted / (float)TotalChunksToMount) * 100.0f;
}


bool UShibChunkingSubsystem::PatchGame()
{
	// make sure the download manifest is up-to-date
	if (bIsDownloadManifestUpToDate)
	{
		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
		// report current chunk status
		for (int32 ChunkID : ChunkDownloadList)
		{
			int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
			UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
		}
		TFunction<void (bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess){OnDownloadComplete(bSuccess);};
		Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);
		
		// start loading mode
		TFunction<void (bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess){OnLoadingModeComplete(bSuccess);};
		Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
		return true;
	}
	
	// you couldn't contact the server to validate your Manifest, so you can't patch
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));
	return false;
}

void UShibChunkingSubsystem::OnLoadingModeComplete(bool bSuccess)
{
	OnDownloadComplete(bSuccess);
}

void UShibChunkingSubsystem::OnMountComplete(bool bSuccess)
{
	OnPatchComplete.Broadcast(bSuccess);
	//LevelOpenFunction(bSuccess);
}

void UShibChunkingSubsystem::OnDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("Download complete"));
		
		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
		FJsonSerializableArrayInt DownloadedChunks;
		
		for (int32 ChunkID : ChunkDownloadList)
		{
			DownloadedChunks.Add(ChunkID);
		}
		//Mount the chunks
		TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess){OnMountComplete(bSuccess);};
		Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);
		OnPatchComplete.Broadcast(true);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Load process failed"));
		// call the delegate
		OnPatchComplete.Broadcast(false);
	}
}

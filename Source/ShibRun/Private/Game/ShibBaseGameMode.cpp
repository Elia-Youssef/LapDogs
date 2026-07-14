// Copyright Shiba Inu Games LLC.

#include "Game/ShibBaseGameMode.h"
#include "EngineUtils.h"
#include "ShibAbility.h"
#include "ShibSessionsEOS.h"
#include "AbilitySystem/ShibAbilitySystemComponent.h"
#include "Actors/ShibRaceManager.h"
#include "Character/ShibCharacter.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibBaseController.h"
#include "GameFramework/PlayerStart.h"
#include "GameplayTags/ShibGameplayTags.h"
#include "Player/ShibBasePlayerState.h"
#include "Player/ShibAiController.h"
#include "Player/ShibLobbyPlayerState.h"

DEFINE_LOG_CATEGORY(LogShibGameMode);

void AShibBaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	MaxInactivePlayers = 0;
}

void AShibBaseGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Look for a race manager in the map
	for (AShibRaceManager* MyRaceManager : TActorRange<AShibRaceManager>(GetWorld()))
	{
		RaceManager = MyRaceManager; // reestablish the linkage to the race manager

		if(AShibGameState* GS = GetGameState<AShibGameState>())
		{
			GS->RaceManager = RaceManager; // cache it in GameState so it's easily accessible client-side
		}
		return; // return after we found one
	}

	// If we didn't find any race manager in the level we spawn one
	if (RaceManagerClass)
	{
		// Spawn our Race manager in the level
		// We want to spawn it once only when we are in the lobby level
		// This actor will be carried over all other levels we go after that uses the same Game mode parent class
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;  // We never want to save race managers into the map

		UWorld* World = GetWorld();
		RaceManager = Cast<AShibRaceManager>(World->SpawnActor(RaceManagerClass, nullptr, nullptr, SpawnInfo));

		if(AShibGameState* GS = GetGameState<AShibGameState>())
		{
			GS->RaceManager = RaceManager; // cache it in GameState so it's easily accessible client-side
		}
	}
}

void AShibBaseGameMode::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	// Add race manager actor to the persistent actor list
	ActorList.Add(RaceManager);
}

void AShibBaseGameMode::Logout(AController* Exiting)
{
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if ( PC != nullptr )
	{
		RemovePlayerControllerFromPlayerCount(PC);
		AddInactivePlayer(PC->PlayerState, PC);

		FGameModeEvents::GameModeLogoutEvent.Broadcast(this, Exiting);
		K2_OnLogout(Exiting);
	}

	// If the list of connected controllers is empty, we shut down the current game server
	if (ConnectedControllers.IsEmpty())
	{
		if (GetShibGI()) ShibGI->TerminateGameSession();
	}
	
	// We completely override the logout function of the base class because we handle unregistering the player before the logout function happened
	//Super::Logout(Exiting);
}

void AShibBaseGameMode::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);

	ConnectedControllers.Remove(OldPC);
	ConnectedControllers.AddUnique(NewPC);
	
	// the old pawn is not being destroyed in the respawn function
	if (APawn* OldPawn = NewPC->GetPawn())
	{
		NewPC->UnPossess();
		OldPawn->Destroy();
	}
}

void AShibBaseGameMode::OnPostLogin(AController* NewPlayer)
{
	if (CanRegisterPlayer(Cast<APlayerController>(NewPlayer)))
	{
		auto* UGameInstance = GetGameInstance();
		auto* SessionsSubsystem = UGameInstance->GetSubsystem<UShibSessionsEOS>();
		auto* InPlayerController = Cast<APlayerController>(NewPlayer);

		if (SessionsSubsystem && IsControllerPlayable(InPlayerController))
		{
			SessionsSubsystem->RegisterPlayer(InPlayerController);
		}
	}

	Super::OnPostLogin(NewPlayer);
}

void AShibBaseGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
}

void AShibBaseGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// We don't call the base class function here, we handle everything ourselves
	//Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!ConnectedControllers.Contains(NewPlayer))
	{
		ConnectedControllers.AddUnique(NewPlayer);
		UE_LOG(LogShibGameMode, Log, TEXT("Starting new player: %d players connected"),ConnectedControllers.Num());
		GetShibGI()->NumConnectedControllers = ConnectedControllers.Num();
	}
	
	if (AShibBaseController* NewShibPlayer = Cast<AShibBaseController>(NewPlayer))
		NewShibPlayer->InitializeShibController();
}

void AShibBaseGameMode::PreLogout(APlayerController* InPlayerController)
{
	auto* UGameInstance = GetGameInstance();
	auto* SessionsSubsystem = UGameInstance->GetSubsystem<UShibSessionsEOS>();

	if (int32 NumbRemovedItems = ConnectedControllers.Remove(InPlayerController); RaceManager &&  NumbRemovedItems>0)
	{
		RaceManager->SetExpectedNumbOfPlayers(ConnectedControllers.Num());
	}

	if (!CanRegisterPlayer(InPlayerController)) return;
	
	if (SessionsSubsystem)
	{
		if (!SessionsSubsystem->UnregisterPlayer(InPlayerController))
		{
			// The player could not be unregistered.
		}
	}
}

void AShibBaseGameMode::KickPlayer(APlayerController* Controller, FText KickReason) const
{
	if (!GameSession || !Controller) return;
	
	UE_LOG(LogShibGameMode, Log, TEXT("kicking player %s. Reason: %s") , *Controller->GetName(), *KickReason.ToString());
	GameSession->KickPlayer(Controller, KickReason);
}

void AShibBaseGameMode::KickPlayers(FText KickReason) const
{
	if (!GameSession) return;
	
	for (APlayerController* Controller : ConnectedControllers)
	{
		KickPlayer(Controller, KickReason);
	}
}

void AShibBaseGameMode::TravelToNextLevel()
{
	ClearGameTimers();

	// If we don't have a race manager we kick all players.
	// The system will destroy the session after the last player has left.
	if (!RaceManager)
	{
		KickPlayers(FText::FromString("Race ended."));
		return;
	}

	// Check if we have another race to play
	// Move players to this level if we have more, or kick everyone if the GP is over
	if (TSoftObjectPtr<UWorld> NextLevel; RaceManager->GetNextGrandPrixLevel(NextLevel) || true) // demo
	{
		// Setup players' start positions for the next race
		// We want to put the first players in the last positions for the next race
		UpdatePlayerStartsByRank(false); // Update real players' starting position
		UpdatePlayerStartsByRank(true); // Update AI players' starting position

		// Set the expected number of player for the next game mode to the amount of connected players
		RaceManager->SetExpectedNumbOfPlayers(ConnectedControllers.Num());
		
		// Move to the next level
		// GetWorld()->ServerTravel(NextLevel.GetAssetName(), true);
		
		// demo
		FString NextLevelName = FMath::RandBool() ? "LVL_LapDogsTrack_01_P" : "LVL_LapDogsTrack_03_P";
		GetWorld()->ServerTravel(NextLevelName, true);
	}
	else
	{
		// GP is over, we kick all players and end the session
		KickPlayers(FText::FromString("Race ended."));
	}
}

void AShibBaseGameMode::EnableShibCharacters(const bool bEnabled, const bool bIgnoreAi)
{
	UShibAbility* PrimaryAbilityFound;
	UShibAbility* SecondaryAbilityFound;
	
	for (const APlayerController* Controller : ConnectedControllers)
	{
		if (AShibCharacter* ShibCharacter = Cast<AShibCharacter>(Controller->GetCharacter()))
		{
			if (bEnabled) // When enabling a character we want to restart abilities
			{
				if (ShibCharacter->Ability->FindAndGetOwnedAbility(TAG_PrimaryAbility, false, PrimaryAbilityFound))
				{
					PrimaryAbilityFound->StartCooldown(ShibCharacter);
				}
				
				if (ShibCharacter->Ability->FindAndGetOwnedAbility(TAG_SecondaryAbility, false, SecondaryAbilityFound))
				{
					SecondaryAbilityFound->StartCooldown(ShibCharacter);
				}
			}
			// Toggle movement for real players' character
			ShibCharacter->GetCharacterMovement()->SetMovementMode(bEnabled? MOVE_Walking : MOVE_None);
		}
	}

	// Ignore AI
	if (bIgnoreAi) return;

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShibAiController::StaticClass(), Actors);
	for(const auto Actor : Actors)
	{
		if(AShibAiController* AiController = Cast<AShibAiController>(Actor))
		{
			if (auto AiShibCharacter = Cast<AShibCharacter>(AiController->GetCharacter()))
			{
				if (bEnabled) // When enabling a character we want to restart abilities
				{
					if (AiShibCharacter->Ability->FindAndGetOwnedAbility(TAG_PrimaryAbility, false, PrimaryAbilityFound))
					{
						PrimaryAbilityFound->StartCooldown(AiShibCharacter);
					}
					
					if (AiShibCharacter->Ability->FindAndGetOwnedAbility(TAG_SecondaryAbility, false, SecondaryAbilityFound))
					{
						SecondaryAbilityFound->StartCooldown(AiShibCharacter);
					}
				}
				// Toggle movement + behavior tree for AI character
				AiShibCharacter->GetCharacterMovement()->SetMovementMode(bEnabled? MOVE_Walking : MOVE_None);
				AiController->StartBehaviorTree(bEnabled);
			}
		}
	}
}

bool AShibBaseGameMode::GetAvailablePlayerStart(const FName CheckpointTag, APlayerStart*& PlayerStartFound)
{
	if (!CheckpointTag.IsNone()) // Get owned player start
	{
		UWorld* World = GetWorld();
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (!Start) continue;
			
			if (Start->PlayerStartTag == CheckpointTag) // return the player start if this is the one we own
			{
				PlayerStartFound = Start;
				return true;
			}
		}
	}
	else // Get random player start
	{
		TArray<AActor*> FoundActors;
		TArray<APlayerStart*> AvailablePlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundActors);
		
		for (auto Actor : FoundActors)
		{
			if (auto FoundPlayerStart = Cast<APlayerStart>(Actor))
			{
				if (!FoundPlayerStart->Tags.Contains(FName("Taken")))
				{
					AvailablePlayerStarts.Add(FoundPlayerStart);
				}
			}
		}

		if (!AvailablePlayerStarts.IsEmpty())
		{
			APlayerStart* RandPlayerStart = AvailablePlayerStarts[FMath::RandRange(0, AvailablePlayerStarts.Num()-1)]; // Random item in the array
			PlayerStartFound = RandPlayerStart;
			return true;
		}
	}
	return false;
}

void AShibBaseGameMode::SetPlayerStartOwner(APlayerStart* Start, AShibBasePlayerState* PsOwner)
{
	if (!Start || !PsOwner) return;
	
	Start->Tags.AddUnique(FName("Taken"));
	
	if (PsOwner)
	{
		PsOwner->CheckpointTag = Start->PlayerStartTag;
		if (PsOwner->IsABot())  // Add this extra tag when this player start is owned by an AI
		{
			Start->Tags.AddUnique(FName("NonPlayerOwner"));
		}
	}
}

void AShibBaseGameMode::RemovePlayerStartOwner(AShibBasePlayerState* PS)
{
	if (!PS || PS->CheckpointTag == FName()) return;

	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* Start = *It;
		if (!Start) continue;
			
		if (PS->CheckpointTag == Start->PlayerStartTag) // return the player start if this is the one we own
		{
			Start->Tags.Empty();
			PS->CheckpointTag = FName();
			return;
		}
	}
}

TSubclassOf<AShibCharacter> AShibBaseGameMode::GetShibClassToSpawn(EShibClass SelectedClass)
{
	// Check if shib class references are valid
	if (!ensureMsgf(GoodBoiClassRef && ZoomyBoiClassRef && ChonkyBoiClassRef && CoolBoiClassRef,
		TEXT("Failed to find one or many Shib Classes. If one of them was moved, please update the reference location in the game mode blueprint."))) return nullptr;
	
	switch (SelectedClass)
	{
	case EShibClass::GoodBoi_Class:
		return GoodBoiClassRef;
	case EShibClass::ZoomyBoi_Class:
		return ZoomyBoiClassRef;
	case EShibClass::ChonkyBoi_Class:
		return ChonkyBoiClassRef;
	case EShibClass::CoolBoi_Class:
		return CoolBoiClassRef;
	}
	return nullptr;
}

void AShibBaseGameMode::RestartPlayer(AController* NewPlayer)
{
	TSubclassOf<APawn> ClassToSpawn;
	APawn* NewCharacter;
	APlayerStart* PlayerStartFound = nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	if (NewPlayer) // If we have a valid controller
	{
		if (auto PS = NewPlayer->GetPlayerState<APlayerState>(); !PS->IsABot()) // Try to spawn a new character for a real player controller
		{
			AShibBaseController* ShibController = Cast<AShibBaseController>(NewPlayer);
			
			if (!ensureAlwaysMsgf(ShibController && ShibController->GetShibBasePS(), TEXT("Unable to respawn Shib Character for player, invalid Shib controller or Shib player state class."))) return;

			ClassToSpawn = GetShibClassToSpawn(ShibController->GetShibBasePS()->SelectedShibClass);
		
			if (!ClassToSpawn)
			{
				UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for player, No valid shib class to spawn."));
				return;
			}

			// Getting spawn location and rotation depending on checkpoints
			if (!GetAvailablePlayerStart(ShibController->GetShibBasePS()->CheckpointTag, PlayerStartFound))
			{
				UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for player, No valid spawn location."));
				return;
			}
			
			NewCharacter = GetWorld()->SpawnActor<APawn>(ClassToSpawn, PlayerStartFound->GetActorLocation(), PlayerStartFound->GetActorRotation(), SpawnParameters);

			// Possess newly created character
			if (NewCharacter)
			{
				SetPlayerStartOwner(PlayerStartFound, ShibController->GetShibBasePS());
				
				// Destroy old character if valid
				if (APawn* OldCharacter = NewPlayer->GetPawn()) {
					NewPlayer->UnPossess();
					OldCharacter->Destroy();
				}
				
				NewPlayer->Possess(NewCharacter);
				
				// Set initial control rotation to starting rotation
				NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);
				FRotator NewControllerRot = PlayerStartFound->GetActorRotation();
				NewControllerRot.Roll = 0.f;
				NewPlayer->SetControlRotation(NewControllerRot);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for player, Failed to spawn the new character pawn."));
			}
		}
		else if (RaceManager && RaceManager->bSpawnAI) // try to spawn a new character for an AI controller
		{
			if (AShibAiController* ShibAiController = Cast<AShibAiController>(NewPlayer)) // Spawn a new character for an AI controller
			{
				if (!ShibAiController->GetShibPS()) return;

				ClassToSpawn = GetShibClassToSpawn(ShibAiController->GetShibPS()->SelectedShibClass);
				
				if (!ClassToSpawn)
				{
					UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for AI, No valid Shib Class to spawn."));
					return;
				}

				// Getting spawn location and rotation depending on checkpoints
				if (!GetAvailablePlayerStart(ShibAiController->GetShibPS()->CheckpointTag, PlayerStartFound))
				{
					UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for AI, No valid spawn location."));
					return;
				}
				
				NewCharacter = GetWorld()->SpawnActor<APawn>(ClassToSpawn, PlayerStartFound->GetActorLocation(), PlayerStartFound->GetActorRotation(), SpawnParameters);

				// Possess newly created character
				if (NewCharacter)
				{
					SetPlayerStartOwner(PlayerStartFound, ShibAiController->GetShibPS());
				
					// Destroy old character if valid
					if (APawn* OldCharacter = NewPlayer->GetPawn()) {
						NewPlayer->UnPossess();
						OldCharacter->Destroy();
					}
					
					NewPlayer->Possess(NewCharacter);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for AI, Failed to spawn the new character pawn."));
				}
			}
		}
	}
	else // Spawn a new character with a random class without any controller, A new default controller will also be spawned for this newly created character
	{
		EShibClass RandomClass = static_cast<EShibClass>(FMath::RandRange(0, 3));
		ClassToSpawn = GetShibClassToSpawn(RandomClass);

		if (!ClassToSpawn)
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character without controller, Invalid random class selected"));
			return;
		}
		
		// Getting spawn location and rotation
		if (!GetAvailablePlayerStart(FName(), PlayerStartFound))
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character for AI, No valid spawn location."));
			return;
		}
		
		NewCharacter = GetWorld()->SpawnActor<APawn>(ClassToSpawn, PlayerStartFound->GetActorLocation(), PlayerStartFound->GetActorRotation(), SpawnParameters);
	
		if (NewCharacter)
		{
			NewCharacter->SpawnDefaultController();
			
			if (AShibBasePlayerState* PS = Cast<AShibBasePlayerState>(NewCharacter->GetPlayerState()))
			{
				PS->SelectedShibClass = RandomClass;

				// Gives AI a random Net ID
				FString RandomString = FGuid::NewGuid().ToString();
				FUniqueNetIdStringRef RandomID = FUniqueNetIdString::Create(RandomString, FName(TEXT("AI")));
				PS->SetUniqueId( *RandomID );
				
				PS->SetPlayerName(GetRandomName());

				SetPlayerStartOwner(PlayerStartFound, PS);

				if (auto AiController = Cast<AShibAiController>(PS->GetOwningController()))
				{
					RaceManager->AIControllers.Add(AiController);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Unable to respawn Shib Character without controller, Failed to spawn the new character pawn."));
		}
	}
}

void AShibBaseGameMode::ClearGameTimers() {}

bool AShibBaseGameMode::IsControllerPlayable(AController* Controller)
{
	const AShibBaseController* ShibController = Cast<AShibBaseController>(Controller);
	// temp fix
	return ShibController != nullptr;
}

// ---- Getters ----

TObjectPtr<AShibBaseController> AShibBaseGameMode::GetShibCtrl()
{
	if (!ShibCtrl) ShibCtrl = Cast<AShibBaseController>(UGameplayStatics::GetPlayerController(this, 0));
	return ShibCtrl;
}

TObjectPtr<UShibGameInstance> AShibBaseGameMode::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

bool AShibBaseGameMode::CanRegisterPlayer(APlayerController* Exiting)
{
	if (Exiting->IsLocalPlayerController() && Exiting->GetLocalPlayer())
	{
		if (Exiting->GetLocalPlayer()->GetPreferredUniqueNetId() == nullptr) return false;
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(Exiting->Player);
		if (RemoteNetConnection == nullptr) return false;
		if (RemoteNetConnection->PlayerId == nullptr) return false;
	}

	return true;
}

void AShibBaseGameMode::UpdatePlayerStartsByRank(const bool bUpdateAiControllers)
{
	AShibGameState* GS = GetGameState<AShibGameState>();
	if (!GS || !RaceManager) return; // Do nothing if we don't have the Game State or the race manager
	if (GS->GetFinalLeaderboard().IsEmpty()) return;
	
	// Setup player start positions for the next race
	// We want to put the first players in the last positions for the next race
	const auto GI = GetShibGI();
	FLeaderboardRow LeaderboardRowFound;

	TArray<AController*> PlayersList = bUpdateAiControllers ? TArray<AController*>(RaceManager->AIControllers) : TArray<AController*>(ConnectedControllers);

	// Set player start for real players
	for (AController* Controller : PlayersList)
	{
		if (AShibBasePlayerState* PS = Cast<AShibBasePlayerState>(Controller->PlayerState))
		{
			if (GS->GetLeaderboardRowByPlayerNetId(PS->GetUniqueId(), LeaderboardRowFound, true))
			{
				// Put the first player last, and the last player first (18 - 1 + 1 = 18)
				int32 NewPosition = GI->DefaultNumConnections - LeaderboardRowFound.PlayerPosition + 1;
				// The player start tag format: Position[Number]
				FString NewStartPosition = TEXT("Position") + FString::FromInt(NewPosition);
				PS->CheckpointTag = FName(*NewStartPosition);
			}
		}
	}
}

#pragma region RandomNames

TSet<FString> AShibBaseGameMode::UsedFirstNames;
TSet<FString> AShibBaseGameMode::UsedLastNames;

FString AShibBaseGameMode::GetRandomName() 
{
	FString FirstName;
	FString LastName;
	
	if (FirstNames.Num() > 0)
	{
		// Get random index and retrieve the first name
		const int32 FirstNameIndex = FMath::RandRange(0, FirstNames.Num() - 1);
		FirstName = FirstNames[FirstNameIndex];

		// Remove the name from the array to avoid duplicates
		FirstNames.RemoveAt(FirstNameIndex);
	}
	else
	{
		// Error where no first name is selected
		FirstName = TEXT("Nameless");
	}

	// Same logic for last name
	if (LastNames.Num() > 0)
	{
		const int32 LastNameIndex = FMath::RandRange(0, LastNames.Num() - 1);
		LastName = LastNames[LastNameIndex];

		// Remove the last name from the array to avoid duplicates
		LastNames.RemoveAt(LastNameIndex);
	}
	else
	{
		// Error where no last name is selected
		LastName = TEXT("Dog");
	}

	return FirstName + TEXT(" ") + LastName; // Combine first and last name
}

#pragma endregion RandomNames
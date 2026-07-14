// Copyright Shiba Inu Games LLC.

#include "Player/ShibPlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "ShibAbility.h"
#include "ShibAbilityComponent.h"
#include "ShibAttributeSetComponent.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibGameMode.h"
#include "Net/UnrealNetwork.h"

AShibPlayerState::AShibPlayerState()
{
	// Create Ability component
	Ability = CreateDefaultSubobject<UShibAbilityComponent>(TEXT("AbilityComponent"));

	// Create Attribute set component
	Attributes = CreateDefaultSubobject<UShibAttributeSetComponent>(TEXT("AbttributeSetComponent"));
}

void AShibPlayerState::BeginPlay()
{
	Super::BeginPlay();


	if (IOnlineSubsystem* OSS = Online::GetSubsystem(this->GetWorld()))
	{
		if (IOnlineIdentityPtr II = OSS->GetIdentityInterface())
		{
			SetUniqueId(FUniqueNetIdRepl{II->CreateUniquePlayerId(FString("Dummy"))});
		}
	}
}

void AShibPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShibPlayerState, Ability);
	DOREPLIFETIME(AShibPlayerState, Attributes);
	DOREPLIFETIME(AShibPlayerState, CurrentLap);
}

bool AShibPlayerState::InitializeAbilitySystemForShibClass()
{
	if (!HasAuthority()) return false; // It needs to be called from the server

	// Return if the ability upgrades are already loaded from the game instance
	if (!bIsAbilitySystemInitialized)
	{
		// Load ability upgrades from Game Instance
		Client_LoadSavedAbilityUpgrades();
		// Mark Ability System initialized to true
		bIsAbilitySystemInitialized = true;
	}

	// Get the ability upgrades and start them to apply their effect on the character's abilities
	for (const auto& AbilityUpgrade : Ability->GetOwnedAbilities())
	{
		Ability->StartAbility(AbilityUpgrade->GetGameplayTag(), this);
	}

	return bIsAbilitySystemInitialized;
}

void AShibPlayerState::UninitializeAbilitySystemForShibClass()
{
	// Cancel all the ability upgrades currently running
	Ability->CancelAllAbilities(this);
}

void AShibPlayerState::IncreaseLapCounter()
{
	// Only the server can change this value
	if (!HasAuthority()) return;

	CurrentLap++;

	// Call the OnRep function directly from the server here to update the host's UI (Listen Server Only)
	// Because OnRep function doesn't fire on the server
	if (GetPawn() && GetPawn()->IsLocallyControlled())
	{
		OnRep_CurrentLap();
	}
}

void AShibPlayerState::SetPlayerIsReadyToRace()
{
	if (bPlayerIsReadyToRace) return;

	if (auto* ShibGM = Cast<AShibGameMode>(GetShibBaseGM()))
	{
		bPlayerIsReadyToRace = true;
		ShibGM->PlayerIsReadyToRace();
	}
}

void AShibPlayerState::Client_LoadSavedAbilityUpgrades_Implementation()
{
	//Apply Ability upgrades saved in the Game Instance if the system was initialized before (Was initilized in a previous map)
	TArray<TSubclassOf<UShibAbility>> AbilityUpgradesList;
	if (UShibGameInstance* GI = GetShibGI())
	{
		if (GI->LoadAbilityUpgrades(AbilityUpgradesList))
		{
			for (TSubclassOf<UShibAbility> AbilityUpgrade : AbilityUpgradesList)
			{
				// Apply ability upgrade
				Ability->AddAbility(AbilityUpgrade, this);
			}
		}
	}
}

void AShibPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState); // This is called so we preserve data chosen to be preserved by default

	auto* NewPlayerState = Cast<AShibPlayerState>(PlayerState);
	if (NewPlayerState == nullptr) return;

	// Save current ability upgrades in the game instance to load them back later with the new player state
	if (GetShibGI()) ShibGI->SaveAbilityUpgrades(Ability->GetOwnedAbilities());
}

void AShibPlayerState::OnRep_CurrentLap()
{
	// We check if the current lap if above one here for a very specific reason. For the first metre of the race where the player has not yet crossed the finish line
	// the lap counter is still set to zero, but we don't want to display the value zero in the UI
	// Lap count set to zero is very important when the race start to calculate the players positions in the first metre of the race before crossing the finish line
	PlayerRaceStatusNotify.Broadcast(CurrentLap > 1 ? CurrentLap : 1);
}

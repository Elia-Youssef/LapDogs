// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTS_UseAbilities.h"

#include "InputActionValue.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShibAiController.h"
#include "Player/ShibPlayerState.h"
#include "Utils/ShibTypes.h"

void UBTS_UseAbilities::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!ShibPS) ShibPS = Cast<AShibPlayerState>(ThisController->PlayerState);
	if (!ShibPS) return;
	
	// check chance to use ability
	if (FMath::FRandRange(0.f, 100.f) > ChanceToUseAbility) return;
	
	// check if there is a closest actor
	ClosestActorInSight = Cast<AActor>(Blackboard->GetValueAsObject(ClosestActorInSightSelector.SelectedKeyName));
	if (!ClosestActorInSight) return;
	
	switch (ShibPS->SelectedShibClass)
	{
	case EShibClass::GoodBoi_Class:
		UsePrimeAbilities();
		break;
	case EShibClass::ZoomyBoi_Class:
		UseRyoAbilities();
		break;
	case EShibClass::ChonkyBoi_Class:
		UseWagmiAbilities();
		break;
	case EShibClass::CoolBoi_Class:
		UseRocketPondAbilities();
		break;
	}
	
	ThisCharacter->InputPickupAbility(FInputActionValue{});
}

// these functions can be condensed to a single function
// later on, we'll be making the npc smarter and not randomly use an ability
// thus the separate function

void UBTS_UseAbilities::UsePrimeAbilities()
{
	if (FMath::RandRange(0, 1) && ClosestActorInSight)
	{
		AimAtTarget();
		ThisCharacter->InputPrimaryAbility(FInputActionValue{});
	} else
	{
		ThisCharacter->InputSecondaryAbility(FInputActionValue{});
	}
}

void UBTS_UseAbilities::UseRyoAbilities()
{
	if (FMath::RandRange(0, 1) && ClosestActorInSight)
	{
		AimAtTarget();
		ThisCharacter->InputPrimaryAbility(FInputActionValue{});
	} else
	{
		ThisCharacter->InputSecondaryAbility(FInputActionValue{});
	}
}

void UBTS_UseAbilities::UseWagmiAbilities()
{
	if (!ClosestActorInSight) return;
	
	AimAtTarget();
		
	if (FMath::RandRange(0, 1))
	{
		ThisCharacter->InputPrimaryAbility(FInputActionValue{});
	} else
	{
		ThisCharacter->InputSecondaryAbility(FInputActionValue{});
	}
}

void UBTS_UseAbilities::UseRocketPondAbilities()
{
	if (FMath::RandRange(0, 1) && ClosestActorInSight)
	{
		AimAtTarget();
		ThisCharacter->InputPrimaryAbility(FInputActionValue{});
	} else
	{
		ThisCharacter->InputSecondaryAbility(FInputActionValue{});
	}
}

void UBTS_UseAbilities::AimAtTarget()
{
	if (!ClosestActorInSight) return;
	
	ThisCharacter->SetActorRotation(
		UKismetMathLibrary::FindLookAtRotation(ThisCharacter->GetActorLocation(), ClosestActorInSight->GetActorLocation())
	);
}

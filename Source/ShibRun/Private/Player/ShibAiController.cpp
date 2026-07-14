// Copyright Shiba Inu Games LLC.

#include "Player/ShibAiController.h"
#include "BrainComponent.h"
#include "ShibAbility.h"
#include "ShibAttributeModifier.h"
#include "ShibAttributeSetComponent.h"
#include "AbilitySystem/ShibAbilitySystemComponent.h"
#include "AI/ShibAi.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "Game/ShibBaseGameMode.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/ShibGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Player/ShibController.h"
#include "Player/ShibBasePlayerState.h"

class AShibGameState;

AShibAiController::AShibAiController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	bWantsPlayerState = true;
	
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	Sight->PeripheralVisionAngleDegrees = 45.f;
	Sight->SetMaxAge(2.f);
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->SetStartsEnabled(true);
	PerceptionComponent->SetDominantSense(*Sight->GetSenseImplementation());
	PerceptionComponent->ConfigureSense(*Sight);
}

void AShibAiController::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AShibAiController::AiTargetPerceptionUpdated);
		PerceptionComponent->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &AShibAiController::AiTargetPerceptionForgotten);
		PerceptionComponent->Activate();
	}
}

void AShibAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_EDITOR
	if(HasAuthority())
	{
		if (!bAllowSimulateAI) return;
		
		// This will ONLY run in editor and never in builds
		StartBehaviorTree(true);
		if (const ACharacter* Char = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* CharMove = Char->GetCharacterMovement())
			{
				CharMove->SetMovementMode(MOVE_Walking);
			}
		}
	}
#endif
}

void AShibAiController::CheckStuck()
{
	UShibAbility* DeadAbility;
	if(GetShibChar()->Ability->FindAndGetOwnedAbility(TAG_Status_Death_Dead, true,DeadAbility) || ! IsBehaviorTreeRunning())
	{
		return;
	}
	
	if(IsStuck())
	{
		UnstuckCharacter();
	}

	if(const APawn* ValidPawn = GetPawn())
	{
		StuckCheckLocation = ValidPawn->GetActorLocation();
	}
}

bool AShibAiController::IsStuck() const
{
	if(StuckCheckLocation == FVector::ZeroVector)
	{
		return false;
	}

	if(const APawn* ValidPawn = GetPawn())
	{
		return FVector::Dist(ValidPawn->GetActorLocation(), StuckCheckLocation) < 100;
	}

	return false;
}

void AShibAiController::UnstuckCharacter()
{
	OnUnstuckCharacter();
}

void AShibAiController::StartBehaviorTree(bool bEnable)
{
	if(BehaviorTree)
	{
		if(bEnable)
		{
			RunBehaviorTree(BehaviorTree);
		}
		else
		{
			StopAIBrain();
		}
	}
}

void AShibAiController::StopAIBrain()
{
	if (BrainComponent)
	{
		BrainComponent->StopLogic("Stop AI Brain");
	}
	
	SetActorTickEnabled(false);
}

bool AShibAiController::IsBehaviorTreeRunning()
{
	const UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(GetBrainComponent());
	if (BTComponent && BTComponent->IsRunning())
	{
		return true;
	}
	
	return false;
}

void AShibAiController::AiTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if( ! Blackboard)
	{
		return;
	}
	
	if (!ClosestActorInSight)
	{
		ClosestActorInSight = Actor;
	}
	else
	{
		const float DistanceToActor = (Actor->GetActorLocation() - GetCharacter()->GetActorLocation()).Length();
		const float DistanceToClosestActor = (ClosestActorInSight->GetActorLocation() - GetCharacter()->GetActorLocation()).Length();
		
		if (DistanceToActor < DistanceToClosestActor) ClosestActorInSight = Actor;
	}

	Blackboard->SetValueAsObject(ShibBlackboardKeys::ClosestActorInSight, ClosestActorInSight);
	ActorsInSight.AddUnique(Actor);
}

void AShibAiController::AiTargetPerceptionForgotten(AActor* Actor)
{
	if (ClosestActorInSight == Actor)
	{
		ClosestActorInSight = nullptr;
		float ClosestDistance = 0;
		for (AActor* A : ActorsInSight)
		{
			const float DistanceToActor = (A->GetActorLocation() - GetCharacter()->GetActorLocation()).Length();
			if (!ClosestActorInSight || DistanceToActor < ClosestDistance)
			{
				ClosestActorInSight = A;
				ClosestDistance = DistanceToActor;
			}
		}
	}
	
	Blackboard->SetValueAsObject(ShibBlackboardKeys::ClosestActorInSight, ClosestActorInSight);
	ActorsInSight.Remove(Actor);
}


// ---- Getters ----

TObjectPtr<UShibGameInstance> AShibAiController::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

TObjectPtr<AShibBasePlayerState> AShibAiController::GetShibPS()
{
	if (!ShibPS) ShibPS = Cast<AShibBasePlayerState>(PlayerState);
	return ShibPS;
}

TObjectPtr<AShibBaseGameMode> AShibAiController::GetShibBaseGM()
{
	if (!ShibBaseGM) ShibBaseGM = Cast<AShibBaseGameMode>(UGameplayStatics::GetGameMode(this));
	return ShibBaseGM;
}

TObjectPtr<AShibCharacter> AShibAiController::GetShibChar()
{
	if (!ShibChar) ShibChar = Cast<AShibCharacter>(GetCharacter());
	return ShibChar;
}

#pragma region ModifySpeed

void AShibAiController::AdjustAiSpeed()
{
	if( ! ModifierSpeedIncreaseClass || ! ModifierSpeedDecreaseClass)
	{
		return;
	}
	
	TMap<APlayerController*, int32> PlayerPositions;
	int32 MyPosition = 0;

	// Getting the player position of this AI
	if(AShibGameState* ShibGameState = Cast<AShibGameState>(GetWorld()->GetGameState()))
	{
		if(GetShibPS()) 
		{
			if(GetShibPS()->GetUniqueId().IsValid())
			{
				FLeaderboardRow Row;
				if (ShibGameState->GetLeaderboardRowByPlayerNetId(GetShibPS()->GetUniqueId(), Row))
				{
					MyPosition = Row.PlayerPosition;
				}
			}
		}
		
		// Getting the player positions of all human players
		for (const auto Controller : GetShibBaseGM()->ConnectedControllers)
		{
			if(AShibController* ShibController = Cast<AShibController>(Controller))
			{
				if(AShibBasePlayerState* ShibPlayerState = Cast<AShibBasePlayerState>(ShibController->PlayerState))
				{
					FLeaderboardRow Row;
					if (ShibGameState->GetLeaderboardRowByPlayerNetId(ShibPlayerState->GetUniqueId(), Row))
					{
						PlayerPositions.Add(Controller, Row.PlayerPosition);
					}
				}
			}
		}
	}
		
	if (PlayerPositions.Num() > 0)
	{
		if(AShibCharacter* ShibCharacterTmp = Cast<AShibCharacter>(GetPawn()))
		{
			#pragma region SortTMap
			TArray<TPair<APlayerController*, int32>> PlayerPositionArray;
			for (const TPair<APlayerController*, int32>& Entry : PlayerPositions)
			{
				PlayerPositionArray.Add(Entry);
			}
			PlayerPositionArray.Sort([](const TPair<APlayerController*, int32>& A, const TPair<APlayerController*, int32>& B) {
				return A.Value < B.Value;  // Sort by PlayerPosition (int32) value
			});
			#pragma endregion SortTMap
			
			// Reset modifier to start
			ShibCharacterTmp->Attributes->RemoveModifier(TAG_Attribute_ShibSpeed, TAG_Modifier_AI_Speed_Increase);
			ShibCharacterTmp->Attributes->RemoveModifier(TAG_Attribute_ShibSpeed, TAG_Modifier_AI_Speed_Decrease);
			ShibCharacterTmp->Attributes->RemoveModifier(TAG_Attribute_ShibAcceleration, TAG_Modifier_AI_Accel);
			ShibCharacterTmp->Attributes->RemoveModifier(TAG_Attribute_ShibFriction, TAG_Modifier_AI_Friction);
			
			if (MyPosition < PlayerPositionArray[0].Value) // Check if the AI is ahead of leading player
			{
				const float Distance = GetDistanceFromPlayer(PlayerPositionArray[0].Key);
				if(Distance > MinDistanceFromPlayer)
				{
					ShibCharacterTmp->Attributes->AddModifier(TAG_Attribute_ShibSpeed, ModifierSpeedDecreaseClass);
				}
			
				// UE_LOG(LogTemp, Warning, TEXT("AI Controller %s is ahead of all players!"), *GetName());
			}
			else if (MyPosition > PlayerPositionArray.Last().Value) // Check if the AI is behind worst player
			{
				const float Distance = GetDistanceFromPlayer(PlayerPositionArray.Last().Key);
				if(Distance > MinDistanceFromPlayer)
				{
					ShibCharacterTmp->Attributes->AddModifier(TAG_Attribute_ShibSpeed, ModifierSpeedIncreaseClass);
					ShibCharacterTmp->Attributes->AddModifier(TAG_Attribute_ShibAcceleration, ModifierAccelIncreaseClass);
					ShibCharacterTmp->Attributes->AddModifier(TAG_Attribute_ShibFriction, ModifierTurnIncreaseClass);
				}
				
				// UE_LOG(LogTemp, Warning, TEXT("AI Controller %s is behind all players!"), *GetName());
			}
		}
	}
}

float AShibAiController::GetDistanceFromPlayer(APlayerController* Player) const
{
	if(const APawn* PawnTmp = Player->GetPawn())
	{
		return FVector::Dist(PawnTmp->GetActorLocation(), GetPawn()->GetActorLocation());
	}
	
	return 0;
}

int32 AShibAiController::GetSpeedModifierCountBasedOnDistance(float Distance)
{
	return FMath::Min(Distance / 1000, 1); // Max 1 modifier. This function was originally intended to give more modifiers
}

#pragma endregion ModifySpeed
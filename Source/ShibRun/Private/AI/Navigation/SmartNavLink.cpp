// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/SmartNavLink.h"

#include "AI/ShibAi.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "Player/ShibAiController.h"

ASmartNavLink::ASmartNavLink()
{
	bSmartLinkIsRelevant = true;
}

void ASmartNavLink::BeginPlay()
{
	Super::BeginPlay();

	OnSmartLinkReached.AddUniqueDynamic(this, &ASmartNavLink::SmartLinkReached);
}

void ASmartNavLink::SmartLinkReached(AActor* MovingActor, const FVector& DestinationPoint)
{
	const ACharacter* Char = Cast<ACharacter>(MovingActor);
	if (!Char) return;

	AShibAiController* Controller = Cast<AShibAiController>(Char->Controller);
	if (!Controller) return;

	UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();
	if (!Blackboard) return;

	if (!Blackboard->GetValueAsBool(ShibBlackboardKeys::Jump))
	{
		Blackboard->SetValueAsBool(ShibBlackboardKeys::Jump, true);
		Blackboard->SetValueAsVector(ShibBlackboardKeys::JumpDestination, DestinationPoint);
	}
}

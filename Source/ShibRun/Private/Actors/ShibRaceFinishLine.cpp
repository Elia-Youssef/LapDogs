// Copyright Shiba Inu Games LLC.

#include "Actors/ShibRaceFinishLine.h"
#include "Character/ShibCharacter.h"
#include "Components/BoxComponent.h"
#include "Game/ShibGameState.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AShibRaceFinishLine::AShibRaceFinishLine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FinishLineCollisionComponent = CreateDefaultSubobject<UBoxComponent>("FinishLineCollisionBox");
	FinishLineCollisionComponent->SetupAttachment(RootComponent);

	FinishLineCollisionComponent->InitBoxExtent(FVector(5.f,300.f,100.f));
	FinishLineCollisionComponent->CanCharacterStepUpOn = ECB_No;
	FinishLineCollisionComponent->SetShouldUpdatePhysicsVolume(false);
	FinishLineCollisionComponent->SetCanEverAffectNavigation(false);
	FinishLineCollisionComponent->bDynamicObstacle = false;

	// Bind event when player collide with the finish line
	FinishLineCollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AShibRaceFinishLine::OnCharacterCrossedFinishLineOverlap);
}

// Called when the game starts or when spawned
void AShibRaceFinishLine::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShibRaceFinishLine::OnCharacterCrossedFinishLineOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// We want to only detect players crossing the finish line on the server
	if (!HasAuthority()) return;

	// Snapshot of the current time, this will be the value compared with the race start time to know how much time the player
	// took to finish the race
	const FDateTime FinishTime = UKismetMathLibrary::UtcNow();

	const AShibCharacter* ShibChar = Cast<AShibCharacter>(OtherActor);
	AShibGameState* const ShibGS = GetWorld() != nullptr ? GetWorld()->GetGameState<AShibGameState>() : nullptr;

	if (ShibChar && ShibGS)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString("Player crossed finish line at: ") + FinishTime.ToString());
		ShibGS->PlayerCrossedFinishLine(ShibChar->GetPlayerState(), FinishTime);
	}
	
}
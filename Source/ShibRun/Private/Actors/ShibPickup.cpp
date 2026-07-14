// Copyright Shiba Inu Games LLC.

#include "Actors/ShibPickup.h"

#include "Character/ShibCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/MovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AShibPickup::AShibPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PickupCollisionComponent = CreateDefaultSubobject<UBoxComponent>("PickupCollisionBox");
	PickupCollisionComponent->SetupAttachment(RootComponent);

	PickupCollisionComponent->InitBoxExtent(FVector(50.f,50.f,50.f));
	PickupCollisionComponent->CanCharacterStepUpOn = ECB_No;
	PickupCollisionComponent->SetCanEverAffectNavigation(false);
	PickupCollisionComponent->SetEnableGravity(false);
	PickupCollisionComponent->Mobility = EComponentMobility::Type::Movable;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetupAttachment(PickupCollisionComponent);

	PickupMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	PickupMesh->CanCharacterStepUpOn = ECB_No;
	PickupMesh->SetCanEverAffectNavigation(false);
	PickupMesh->SetEnableGravity(false);
	PickupMesh->Mobility = EComponentMobility::Type::Movable;

	PickupMeshNiagara = CreateDefaultSubobject<UNiagaraComponent>("PickupMeshNiagara");
	PickupMeshNiagara->SetupAttachment(PickupMesh);
	PickupMeshNiagara->bAutoActivate = true;
	
	OnPickupNiagara = CreateDefaultSubobject<UNiagaraComponent>("OnPickupNiagara");
	OnPickupNiagara->SetupAttachment(PickupCollisionComponent);
	OnPickupNiagara->bAutoActivate = false;
	
	PickupMovementComponent = CreateDefaultSubobject<UMovementComponent>("PickupMovementComp");
	if (PickupMovementComponent)
	{
		PickupMovementComponent->UpdatedComponent = PickupCollisionComponent;
	}

	// Bind event when player collide with the finish line
	PickupCollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AShibPickup::OnCharacterPickupOverlap);
}

// Called when the game starts or when spawned
void AShibPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShibPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TickTimePassed += DeltaSeconds;

	if (bRotate)
	{
		AddActorLocalRotation(FRotator(0, DeltaSeconds * RotationSpeed, 0));
	}
	
	if (bOscillate)
	{
		float TimeSin = FMath::Sin(TickTimePassed * OscillationSpeed);
		AddActorLocalOffset(FVector(0, 0, TimeSin * OscillationDistance));
	}
}

void AShibPickup::OnCharacterPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto* Shib = Cast<AShibCharacter>(OtherActor))
	{
		if (OnPickupNiagara) OnPickupNiagara->Activate();
		
		OnPickup(Shib);
	}
}

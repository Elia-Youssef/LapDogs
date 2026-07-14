// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShibRaceFinishLine.generated.h"

class UBoxComponent;

UCLASS()
class SHIBRUN_API AShibRaceFinishLine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShibRaceFinishLine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnCharacterCrossedFinishLineOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	/** The Collision Box component being used to detect when a player cross the finish line. */
	UPROPERTY(Category=ShibFinishLine, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> FinishLineCollisionComponent;
};

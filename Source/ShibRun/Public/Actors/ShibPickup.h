// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "ShibPickup.generated.h"

class AShibCharacter;
class UBoxComponent;
class UMovementComponent;

UCLASS()
class SHIBRUN_API AShibPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShibPickup();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bRotate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Rotation", meta=(EditCondition="bRotate", EditConditionHides))
	float RotationSpeed = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bOscillate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Oscillation", meta=(EditCondition="bOscillate", EditConditionHides))
	float OscillationSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Oscillation", meta=(EditCondition="bOscillate", EditConditionHides))
	float OscillationDistance = 1.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnCharacterPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPickup(AShibCharacter* OtherActor);
	
private:	
	/** Movement component used for movement logic of the pickup actor */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UMovementComponent> PickupMovementComponent;

	/** The Collision Box component being used to detect when a hit the pickup actor. */
	UPROPERTY(Category=ShibFinishLine, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> PickupCollisionComponent;

	/** The main mesh associated with this pickup actor */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> PickupMeshNiagara;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> OnPickupNiagara;

	float TickTimePassed;
};

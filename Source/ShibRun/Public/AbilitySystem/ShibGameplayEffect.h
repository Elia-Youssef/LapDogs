// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAbilityEffect.h"
#include "UObject/Object.h"
#include "ShibGameplayEffect.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UShibGameplayEffect : public UShibAbilityEffect
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AShibCharacter* GetOwningShibCharacter();
	
	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Gameplay", meta=(BlueprintSpawnableComponent, DisplayName="Spawn Actor From Class", AdvancedDisplay="SpawnCollisionHandlingOverride,TransformScaleMethod,Owner,Instigator", DeterminesOutputType="ClassToSpawn"))
	AActor* SpawnActorFromClass(TSubclassOf<AActor> ClassToSpawn, const FVector& Location, const FRotator& Rotation, ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator);

protected:
	TObjectPtr<AShibCharacter> OwningShibCharacter = nullptr;
};

// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAbility.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "ShibGameplayAbility.generated.h"

class AShibCharacter;

UCLASS()
class SHIBRUN_API UShibGameplayAbility : public UShibAbility
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AShibCharacter* GetOwningShibCharacter();
	
	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Trace")
	bool SphereOverlapActors(const FVector SpherePos, float SphereRadius, const TArray<TEnumAsByte<EObjectTypeQuery> > & ObjectTypes, UClass* ActorClassFilter, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Trace")
	bool LineTraceSingleForObjects(const FVector Start, const FVector End, const TArray<TEnumAsByte<EObjectTypeQuery> > & ObjectTypes, const TArray<AActor*>& ActorsToIgnore, FHitResult& OutHit, bool bIgnoreSelf);

	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Trace")
	bool LineTraceMultiForObjects(const FVector Start, const FVector End, const TArray<TEnumAsByte<EObjectTypeQuery> > & ObjectTypes, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHit, bool bIgnoreSelf);
	
	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Gameplay", meta=(BlueprintSpawnableComponent, DisplayName="Spawn Actor From Class", AdvancedDisplay="SpawnCollisionHandlingOverride,TransformScaleMethod,Owner,Instigator", DeterminesOutputType="ClassToSpawn"))
	AActor* SpawnActorFromClass(TSubclassOf<AActor> ClassToSpawn, const FVector& Location, const FRotator& Rotation, ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator);

	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Gameplay",  meta=(DisplayName = "Get All Actors Of Class", DeterminesOutputType="ActorClass", DynamicOutputParam="OutActors"))
	static void GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable, Category="GameplayAbility|Gameplay", meta=(DeterminesOutputType="ActorClass"))
	static AActor* GetActorOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);

protected:
	TObjectPtr<AShibCharacter> OwningShibCharacter = nullptr;
};

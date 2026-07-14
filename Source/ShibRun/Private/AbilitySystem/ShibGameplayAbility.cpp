// Copyright Shiba Inu Games LLC.

#include "AbilitySystem/ShibGameplayAbility.h"
#include "EngineUtils.h"
#include "KismetTraceUtils.h"
#include "ShibAbilityComponent.h"
#include "Character/ShibCharacter.h"
#include "Projectiles/ShibProjectileBase.h"

AShibCharacter* UShibGameplayAbility::GetOwningShibCharacter()
{
	if (OwningShibCharacter)
	{
		return OwningShibCharacter;
	}

	if (auto* ShibAbilityComponent = GetOwningComponent())
	{
		OwningShibCharacter = Cast<AShibCharacter>(ShibAbilityComponent->GetAvatarActor());
	}
	
	return OwningShibCharacter;
}

bool UShibGameplayAbility::SphereOverlapActors(const FVector SpherePos, float SphereRadius, const TArray<TEnumAsByte<EObjectTypeQuery> > & ObjectTypes, UClass* ActorClassFilter, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors)
{
	if(const UWorld* World = GetWorld())
	{
		return UKismetSystemLibrary::SphereOverlapActors(World, SpherePos, SphereRadius, ObjectTypes, ActorClassFilter, ActorsToIgnore, OutActors);
	}
	
	return false;
}

bool UShibGameplayAbility::LineTraceSingleForObjects(const FVector Start,
	const FVector End, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore,
	FHitResult& OutHit, bool bIgnoreSelf)
{
	if(const UWorld* World = GetWorld())
	{
		return UKismetSystemLibrary::LineTraceSingleForObjects(World, Start, End, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, bIgnoreSelf);
	}
	
	return false;
}

AActor* UShibGameplayAbility::SpawnActorFromClass(TSubclassOf<AActor> ClassToSpawn, const FVector& Location, const FRotator& Rotation, ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride, ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator)
{
		if (!GetWorld())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid world context"));
			return nullptr;
		}
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = SpawnCollisionHandlingOverride;
		SpawnParameters.TransformScaleMethod = TransformScaleMethod;
		SpawnParameters.Owner = Owner;
		SpawnParameters.Instigator = Instigator;
		

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, Location, Rotation, SpawnParameters);
		SpawnParameters.Template = SpawnedActor;
	
		return SpawnedActor;
}

void UShibGameplayAbility::GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass,
	TArray<AActor*>& OutActors)
{
	OutActors.Reset();

	// We do nothing if no is class provided, rather than giving ALL actors!
	if (ActorClass)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
			{
				AActor* Actor = *It;
				OutActors.Add(Actor);
			}
		}
	}
}

AActor* UShibGameplayAbility::GetActorOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass)
{
	// We do nothing if no is class provided
	if (ActorClass)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
			{
				AActor* Actor = *It;
				return Actor;
			}
		}
	}

	return nullptr;
}

bool UShibGameplayAbility::LineTraceMultiForObjects(const FVector Start,
                                                    const FVector End, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore,
                                                    TArray<FHitResult>& OutHits, bool bIgnoreSelf)
{
	if(const UWorld* World = GetWorld())
	{
		return UKismetSystemLibrary::LineTraceMultiForObjects(World, Start, End, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, bIgnoreSelf);
	}
	
	return false;
}

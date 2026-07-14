// Copyright Shiba Inu Games LLC.


#include "AbilitySystem/ShibGameplayEffect.h"

#include "Character/ShibCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

AShibCharacter* UShibGameplayEffect::GetOwningShibCharacter()
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

AActor* UShibGameplayEffect::SpawnActorFromClass(TSubclassOf<AActor> ClassToSpawn, const FVector& Location,
	const FRotator& Rotation, ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride,
	ESpawnActorScaleMethod TransformScaleMethod, AActor* Owner, APawn* Instigator)
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
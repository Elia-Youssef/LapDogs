// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShibAttributeModifier.h"
#include "Perception/AIPerceptionTypes.h"
#include "ShibAiController.generated.h"

class UShibAbility;
class AShibCharacter;
class UAISenseConfig_Sight;
class AShibBaseGameMode;
class AShibBasePlayerState;
class UShibGameInstance;
/**
 * 
 */
UCLASS(BlueprintType)
class SHIBRUN_API AShibAiController : public AAIController
{
	GENERATED_BODY()

public:
	AShibAiController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// Game
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibGameInstance> ShibGI;
	TObjectPtr<UShibGameInstance> GetShibGI();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibBasePlayerState> ShibPS;
	TObjectPtr<AShibBasePlayerState> GetShibPS();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibBaseGameMode> ShibBaseGM;
	TObjectPtr<AShibBaseGameMode> GetShibBaseGM();
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibCharacter> ShibChar;
	TObjectPtr<AShibCharacter> GetShibChar();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AiSettings")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/**
	 * This allows AI dogs to instantly run, ignoring any countdown from the game mde.
	 */
	UPROPERTY(EditDefaultsOnly, Category="AiSettings")
	bool bAllowSimulateAI = false;
	
	// AI Perception
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAISenseConfig_Sight> Sight;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> ActorsInSight;

	UPROPERTY(BlueprintReadOnly)
	AActor* ClosestActorInSight;

	FVector StuckCheckLocation = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable)
	void CheckStuck();

	bool IsStuck() const;

	void UnstuckCharacter();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnUnstuckCharacter();

	void StartBehaviorTree(bool bEnable);

	/**Permanently shuts down the AI from using abilities and movement.*/
	void StopAIBrain();

	UFUNCTION(BlueprintCallable)
	bool IsBehaviorTreeRunning();

#pragma region ModifySpeed

	/**The minimum distance in cm/units the AI has to be away from a player to have speed modifications.*/
	UPROPERTY(EditDefaultsOnly, Category="AiSettings|Movement")
	float MinDistanceFromPlayer = 3500.f;

	UPROPERTY(EditDefaultsOnly, Category="AiSettings|Movement")
	TSubclassOf<UShibAttributeModifier> ModifierSpeedIncreaseClass;

	UPROPERTY(EditDefaultsOnly, Category="AiSettings|Movement")
	TSubclassOf<UShibAttributeModifier> ModifierAccelIncreaseClass;

	UPROPERTY(EditDefaultsOnly, Category="AiSettings|Movement")
	TSubclassOf<UShibAttributeModifier> ModifierTurnIncreaseClass;

	UPROPERTY(EditDefaultsOnly, Category="AiSettings|Movement")
	TSubclassOf<UShibAttributeModifier> ModifierSpeedDecreaseClass;

	UFUNCTION(BlueprintCallable)
	void AdjustAiSpeed();

	/**This will only return player controllers (not AI) belonging to the same Lap.*/
	UFUNCTION(BlueprintPure)
	float GetDistanceFromPlayer(APlayerController* PLayer) const;

	/**Not in use. It was originally intended for giving multiple modifiers depending on distance.*/
	int32 GetSpeedModifierCountBasedOnDistance(float Distance);
	
#pragma endregion ModifySpeed

protected:
	UFUNCTION()
	void AiTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void AiTargetPerceptionForgotten(AActor* Actor);
	
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
};

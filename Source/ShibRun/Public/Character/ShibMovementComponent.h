// Copyright Shiba Inu Games LLC.

#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "ShibMovementComponent.generated.h"

class AShibCharacter;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Drift			UMETA(DisplayName = "Drift"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class UShibMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:

	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	virtual void BeginPlay() override;

	virtual float GetMaxAcceleration() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	UCurveFloat* AccelerationCurve;
	
	
#pragma region Flags

	class FSavedMove_Sy : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToDrift:1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};
	
	class FNetworkPredictionData_Client_Sy : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Sy(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;
		
		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
	bool Safe_bWantsToDrift;
	
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

#pragma endregion Flags

#pragma region CustomMovement

	FVector PhysDirection = FVector::ZeroVector;
	float PhysInitialSpeed = 0.f;
	float PhysElapsedTime = 0.f;
	float PhysDistance = 0.f;
	
	void ProcessMovementAgainstSurface(float deltaTime);
	
	void NewProcessMovement(float deltaTime);

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	UFUNCTION(BlueprintPure)
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) override;

	/**Checks surface beneath the character at a custom distance.*/
	FHitResult CheckFloor(float Distance) const;

#pragma region CustomMovementModes

	bool bWantsToDrift = false;
	
	UPROPERTY(EditAnywhere, Category="Drift")
	float DriftFactor = 1.f;

	/**Higher value means drifting slows down movement speed more.*/
	UPROPERTY(EditAnywhere, Category="Drift")
	float DriftDecelerate = 1.f;

	UFUNCTION(BlueprintCallable)
	void TryEnterDrift();

	UFUNCTION(BlueprintCallable)
	void TryExitDrift();

	bool CanDrift() const;

	bool CanExitDrift() const;
	
	void EnterDrift();

	UFUNCTION(BlueprintCallable)
	void ExitDrift();

	void PhysDrift(float deltaTime, int32 Iterations);
	
#pragma endregion CustomMovementModes

#pragma endregion CustomMovement
	
	
};

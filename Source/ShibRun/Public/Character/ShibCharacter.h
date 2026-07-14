// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "ShibCharacter.generated.h"

class UShibGameInstance;
class AShibController;
class UAISenseConfig_Sight;
class UAIPerceptionComponent;
class UShibHealthComponent;
class AShibPlayerState;
class UWidgetComponent;
class UShibInputConfig;
class UShibAbilitySystemComponent;
class UShibAttributeSetComponent;
class UShibAbility;
class UShibAttribute;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterDiedDelegate, AActor*, DeadActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeforeLaunchCharacterSignature);

UENUM(BlueprintType)
enum ECustomMovementDisplay
{
	CMD_None,
	CMD_Drifting,
};

UCLASS()
class SHIBRUN_API AShibCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShibCharacter();

	UCameraComponent* GetCameraComponent() const{ return CameraComponent; }
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnRep_PlayerState() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to set replicated components
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Update the name above the player
	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePlayerNameWidget();

	// Event called when the player dies
	UPROPERTY(BlueprintAssignable, Category = "ShibCharacter")
	FCharacterDiedDelegate OnCharacterDeath;

	// Ability component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UShibAbilitySystemComponent> Ability;

	// Attribute Set component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UShibAttributeSetComponent> Attributes;

	// Health component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UShibHealthComponent> HealthComponent;

	/**Controls how fast the dog will turn. This is not safe from cheating because it's a local restriction and not part of the movement comp.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Movement", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CameraMaxRotationRate = 1.2;

	/**Minimum camera rotation rate when reaching high speed.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Movement", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float CameraMinRotationRate = 0.2;
	
	// Default Input functions
	UFUNCTION(BlueprintNativeEvent)
	void InputPrimaryAbility(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputSecondaryAbility(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputPickupAbility(const FInputActionValue& InputActionValue);

#pragma region CustomMovementMode

	/**Specific custom movement modes are not replicated by default and flags are only relevant to Owner and Server.
	 * This replicated enum (SkipOwner) is to let other clients know about our custom movement mode, for animation ect.
	 */
	UPROPERTY(Replicated)
	TEnumAsByte<ECustomMovementDisplay> CustomMovementDisplay = CMD_None;

	void SetCustomMovementDisplay(ECustomMovementDisplay NewCustomMovementDisplay);

	UFUNCTION(BlueprintPure)
	TEnumAsByte<ECustomMovementDisplay> GetCustomMovementDisplay() const;

#pragma endregion CustomMovementMode

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	// Default Input functions
	UFUNCTION(BlueprintNativeEvent)
	void InputMove(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputLook(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputJump(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputStartDrift(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputStopDrift(const FInputActionValue& InputActionValue);
	UFUNCTION(BlueprintNativeEvent)
	void InputRespawn(const FInputActionValue& InputActionValue);

	// Init Player character
	void InitPlayer();
	
	// Uninit Player character
	void UninitPlayer();
	
	UFUNCTION()
	void OnMovementSpeedChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnAccelerationChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnFrictionChanged(float OldValue, float NewValue);
	
	/**
	* When the character death execution start
	* This is important to call this before actually destroying the actor
	* Bound to an event in the health component
	*/
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter")
	virtual void HandleOnShibDeathStarted(AActor* OwningActor);

	/**
	* When the character death execution is done
	* This is used to actually destroyed the character
	* Bound to an event in the health component
	*/
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter")
	virtual void HandleOnShibDeathFinished(AActor* OwningActor);
	
	// Disable movement and collision from character
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter")
	void SetMovementAndCollision(bool bEnable);

	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBeforeLaunchCharacterSignature OnBeforeLaunchCharacterDelegate;

	/**This is used to counteract framerate changing turn speed.*/
	float LastDeltaSeconds = 0.f;

	UFUNCTION()

#pragma region Abilities
	
	// Initialize the Character's attributes and abilities 
	void InitializeAbilitySystem();
	
	// Uninitialize the Character's attributes and abilities 
	void UninitializeAbilitySystem();

#pragma endregion Abilities

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent = nullptr;

	// Custom Input config
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UShibInputConfig> InputConfig;

	// Character Primary Ability Tag
	UPROPERTY(BlueprintReadOnly, Replicated)
	FGameplayTag PrimaryAbilityTag;
	
	// Character Secondary Ability Tag
	UPROPERTY(BlueprintReadOnly, Replicated)
	FGameplayTag SecondaryAbilityTag;
	
	// Player Name Widget Component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> PlayerNameWidget;
	
	UPROPERTY()
	TObjectPtr<AShibPlayerState> ShibPS;
	TObjectPtr<AShibPlayerState> GetShibPS();

	UPROPERTY()
	TObjectPtr<UShibGameInstance> ShibGI;
	TObjectPtr<UShibGameInstance> GetShibGI();
	
};

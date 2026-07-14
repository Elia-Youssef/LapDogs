// Copyright Shiba Inu Games LLC.

#include "Character/ShibCharacter.h"
#include "ShibAttribute.h"
#include "ShibAttributeSetComponent.h"
#include "Components/WidgetComponent.h"
#include "GameplayTags/ShibGameplayTags.h"
#include "Inputs/ShibTaggedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/ShibPlayerState.h"
#include "ShibAbility.h"
#include "Character/ShibHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShibBaseController.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ShibAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Game/ShibGameInstance.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AShibCharacter::AShibCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	//need to take off blueprint camera
	SpringArmComponent = CreateOptionalDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));

	if (SpringArmComponent.Get())
	{
		SpringArmComponent->SetupAttachment(RootComponent);
		
		// We don't want to control character rotation with our camera
		// this is why we disable these settings
		SpringArmComponent->bUsePawnControlRotation = false;
		SpringArmComponent->bInheritPitch = false;
		SpringArmComponent->bInheritRoll = false;
		SpringArmComponent->bInheritYaw = true;

		SpringArmComponent->bEnableCameraLag = true;
		SpringArmComponent->CameraLagSpeed = 5.0f;
		
		CameraComponent = CreateOptionalDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
		CameraComponent->SetupAttachment(SpringArmComponent.Get(), USpringArmComponent::SocketName);
	}
	
	// Create Ability component
	Ability = CreateDefaultSubobject<UShibAbilitySystemComponent>(TEXT("AbilityComponent"));

	// Create Attribute set component
	Attributes = CreateDefaultSubobject<UShibAttributeSetComponent>(TEXT("AbttributeSetComponent"));

	// Create Health component
	HealthComponent = CreateDefaultSubobject<UShibHealthComponent>(TEXT("HealthComponent"));
	
	PlayerNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameWidget"));
	PlayerNameWidget->SetupAttachment(GetMesh());
}

void AShibCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Define replicated components here...
	
	DOREPLIFETIME_CONDITION(AShibCharacter, PrimaryAbilityTag, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AShibCharacter, SecondaryAbilityTag, COND_OwnerOnly);
	
	DOREPLIFETIME_CONDITION(AShibCharacter, CustomMovementDisplay, COND_SkipOwner);
}

void AShibCharacter::BeginPlay()
{
	Super::BeginPlay();

    //TODO Remove this if/when we go away from control rotation based turning
	if(AController* NewController = GetController())
	{
		NewController->SetControlRotation(GetActorRotation());
	}
}

void AShibCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LastDeltaSeconds = DeltaSeconds;
}

void AShibCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Uninitialize player before getting destroyed
	UninitPlayer();
	
	Super::EndPlay(EndPlayReason);
}

void AShibCharacter::PossessedBy(AController* NewController)
{
	// This runs on server right after getting a player controller at begin play
	// We initialize player stuff on the server side here...

	Super::PossessedBy(NewController);

	//UE_LOG(LogTemp, Log, TEXT("Server PossessedBy"));
	
	InitPlayer();

	InitializeAbilitySystem();
}

void AShibCharacter::UnPossessed()
{
	Super::UnPossessed();
}

void AShibCharacter::OnRep_PlayerState()
{
	// This runs on client right after getting a player state at begin play
	// We initialize player stuff on the client side here...

	Super::OnRep_PlayerState();

	//UE_LOG(LogTemp, Log, TEXT("Client Onrep Player State"));
	
	InitPlayer();
}

void AShibCharacter::HandleOnShibDeathStarted(AActor* OwningActor)
{
	OnCharacterDeath.Broadcast(this);
}

void AShibCharacter::HandleOnShibDeathFinished(AActor* OwningActor)
{
	if (HasAuthority())
	{
		// Reset Health when we're back to life
		HealthComponent->ResetHealth();
	}
	// Reenable movement
	SetMovementAndCollision(true);
}

void AShibCharacter::OnMovementSpeedChanged(float OldValue, float NewValue)
{
	auto* Movement = GetCharacterMovement();
	if (!ensureAlwaysMsgf(Movement, TEXT("No character movement your Character is ill formed."))) return;
	Movement->MaxWalkSpeed = NewValue;
}

void AShibCharacter::OnAccelerationChanged(float OldValue, float NewValue)
{
	auto* Movement = GetCharacterMovement();
	if (!ensureAlwaysMsgf(Movement, TEXT("No character movement your Character is ill formed."))) return;
	Movement->MaxAcceleration = NewValue;
}

void AShibCharacter::OnFrictionChanged(float OldValue, float NewValue)
{
	auto* Movement = GetCharacterMovement();
	if (!ensureAlwaysMsgf(Movement, TEXT("No character movement your Character is ill formed."))) return;
	Movement->GroundFriction = NewValue;
}

void AShibCharacter::InputMove_Implementation(const FInputActionValue& InputActionValue)
{	
	const auto MoveVector = InputActionValue.Get<FVector2d>();

	if (Controller == nullptr || MoveVector.SquaredLength() == 0) return;

	//Get the forward direction from the controller
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
	// const FVector RightDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	AddMovementInput(ForwardDirection, MoveVector.Y);
	// AddMovementInput(RightDirection, -MoveVector.X);
	
	// FRotator DesiredRotation = GetActorRotation();
	// DesiredRotation.Yaw += MoveVector.X * TurnRate;  // TurnRate is a configurable multiplier
	// SetActorRotation(DesiredRotation);
	
	// This controls how fast the dog turns but it is not safe from hacking - it's a local restriction and not part of the movement comp
	// The rotation rate is based on current speed
	const float SpeedFactor = FMath::Clamp(1.0f - (GetVelocity().Size() / 1500), CameraMinRotationRate, 1.0f); // 1500 is an aprox value for max speed
	const float AdjustedYawInput = -MoveVector.X * CameraMaxRotationRate * (SpeedFactor * (LastDeltaSeconds * 80));
	AddControllerYawInput(AdjustedYawInput);

}

void AShibCharacter::InputLook_Implementation(const FInputActionValue& InputActionValue)
{
	// float Sensitivity = 1;
	// float YAxis = 1;
	// if (const TObjectPtr<UShibSaveSubsystem> Save = GetShibGI()->SaveSubsystem)
	// {
	// 	Sensitivity = Save->ShibSettingsSave->MouseSensitivity;
	// 	YAxis = Save->ShibSettingsSave->bInvertYAxis ? -1 : 1;
	// }
	//
	// const auto LookVector = InputActionValue.Get<FVector2d>();
	// AddControllerYawInput(LookVector.X * Sensitivity);
	// AddControllerPitchInput(-LookVector.Y * Sensitivity * YAxis);
}

void AShibCharacter::InputJump_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StartAbility(TAG_JumpAbility, this);
}

void AShibCharacter::InputPrimaryAbility_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StartAbility(PrimaryAbilityTag, this);
}

void AShibCharacter::InputSecondaryAbility_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StartAbility(SecondaryAbilityTag, this);
}

void AShibCharacter::InputPickupAbility_Implementation(const FInputActionValue& InputActionValue)
{
	UShibAbility* PickupAbility;
	if (Ability->FindAndGetOwnedAbility(TAG_PickupAbility, false,PickupAbility))
	{
		Ability->StartAbility(PickupAbility->GetGameplayTag(), this);
	}
}

void AShibCharacter::InputStartDrift_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StartAbility(TAG_DriftAbility, this);
}

void AShibCharacter::InputStopDrift_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StopAbility(TAG_DriftAbility, this);
}

void AShibCharacter::InputRespawn_Implementation(const FInputActionValue& InputActionValue)
{
	Ability->StartAbility(TAG_RespawnAbility, this);
}

void AShibCharacter::InitPlayer()
{
	if (GetShibPS())
	{
		ShibPS->Ability->InitAbilityActorInfo(ShibPS, this);
	}

	// Init health component to let it know what are our ability and attribute set component
	HealthComponent->InitializeWithAbilitySystem(Ability, Attributes);
	
	if (!HealthComponent->OnDeathStarted.IsAlreadyBound(this, &AShibCharacter::HandleOnShibDeathStarted))
		HealthComponent->OnDeathStarted.AddDynamic(this, &AShibCharacter::HandleOnShibDeathStarted);

	if (!HealthComponent->OnDeathFinished.IsAlreadyBound(this, &AShibCharacter::HandleOnShibDeathFinished))
		HealthComponent->OnDeathFinished.AddDynamic(this, &AShibCharacter::HandleOnShibDeathFinished);
	
	// Set event to handle movement speed change based on the Shib Speed attribute
	auto* SpeedAttr = Attributes->GetAttribute(TAG_Attribute_ShibSpeed, true);
	if (!ensureAlwaysMsgf(SpeedAttr, TEXT("No ShibSpeed attribute, your Character is ill formed."))) return;

	auto* Movement = GetCharacterMovement();
	if (!ensureAlwaysMsgf(Movement, TEXT("No character movement your Character is ill formed."))) return;
	if (!SpeedAttr->CurrentValueChanged.IsAlreadyBound(this, &AShibCharacter::OnMovementSpeedChanged))
		SpeedAttr->CurrentValueChanged.AddDynamic(this, &AShibCharacter::OnMovementSpeedChanged);
	OnMovementSpeedChanged(Movement->MaxWalkSpeed, SpeedAttr->GetBaseValue());

	// Set event to handle movement speed change based on the Shib Acceleration attribute
	auto* AccelAttr = Attributes->GetAttribute(TAG_Attribute_ShibAcceleration, true);
	if (!ensureAlwaysMsgf(AccelAttr, TEXT("No ShibAcceleration attribute, your Character is ill formed."))) return;
	
	if (!AccelAttr->CurrentValueChanged.IsAlreadyBound(this, &AShibCharacter::OnAccelerationChanged))
		AccelAttr->CurrentValueChanged.AddDynamic(this, &AShibCharacter::OnAccelerationChanged);
	OnAccelerationChanged(Movement->MaxAcceleration, AccelAttr->GetBaseValue());

	// Set event to handle movement speed change based on the Shib Friction attribute
	auto* FrictionAttr = Attributes->GetAttribute(TAG_Attribute_ShibFriction, true);
	if (!ensureAlwaysMsgf(FrictionAttr, TEXT("No ShibFriction attribute, your Character is ill formed."))) return;
	
	if (!FrictionAttr->CurrentValueChanged.IsAlreadyBound(this, &AShibCharacter::OnFrictionChanged))
		FrictionAttr->CurrentValueChanged.AddDynamic(this, &AShibCharacter::OnFrictionChanged);
	OnFrictionChanged(Movement->GroundFriction, FrictionAttr->GetBaseValue());
}

void AShibCharacter::UninitPlayer()
{
	// Remove bound function from attributes
	if (auto* SpeedAttr = Attributes->GetAttribute(TAG_Attribute_ShibSpeed, true))
	{
		SpeedAttr->CurrentValueChanged.RemoveAll(this);	
	}
	
	if (auto* AccelAttr = Attributes->GetAttribute(TAG_Attribute_ShibAcceleration, true))
	{
		AccelAttr->CurrentValueChanged.RemoveAll(this);
	}

	if (auto* FrictionAttr = Attributes->GetAttribute(TAG_Attribute_ShibFriction, true))
	{
		FrictionAttr->CurrentValueChanged.RemoveAll(this);
	}

	HealthComponent->OnDeathStarted.RemoveAll(this);
	HealthComponent->OnDeathFinished.RemoveAll(this);
	
	UninitializeAbilitySystem();
}

void AShibCharacter::SetMovementAndCollision(bool bEnable)
 {
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);

	auto* MoveComp = GetCharacterMovement();
	check(MoveComp);
	
	 if (bEnable)
	 {
	 	if (Controller)
	 	{
	 		Controller->ResetIgnoreMoveInput();
	 	}

	 	CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	 }
	 else
	 {
	 	if (Controller)
	 	{
	 		Controller->SetIgnoreMoveInput(true);
	 	}

	 	CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	 }
}

void AShibCharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	OnBeforeLaunchCharacterDelegate.Broadcast();
	
	Super::LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);
}

void AShibCharacter::InitializeAbilitySystem()
{
	if (!HasAuthority()) return;
	
	UE_LOG(LogTemp, Log, TEXT("Character Abilty System initialization"));
	
	// Init ability system on the player state to receive any ability upgrades
	if (GetShibPS())
	{
		ShibPS->InitializeAbilitySystemForShibClass();	
	}
	
	// Get ability gameplay tags for the current possessed class
	for (auto OwnedAbility : Ability->GetOwnedAbilities())
	{
		// Get primary ability gameplay tag for later use
		if (OwnedAbility->GetGameplayTag().MatchesTag(TAG_PrimaryAbility))
		{
			PrimaryAbilityTag = OwnedAbility->GetGameplayTag();
			UE_LOG(LogTemp, Log, TEXT("Primary ability added: %s"), *PrimaryAbilityTag.ToString());
		}
		// Get secondary ability gameplay tag for later use
		else if (OwnedAbility->GetGameplayTag().MatchesTag(TAG_SecondaryAbility))
		{
			SecondaryAbilityTag = OwnedAbility->GetGameplayTag();
			UE_LOG(LogTemp, Log, TEXT("Secondary ability added: %s"), *SecondaryAbilityTag.ToString());
		}
	}
}

void AShibCharacter::UninitializeAbilitySystem()
{
	if (GetShibPS())
	{
		// ask the player state to stop ability upgrades when this character is destroyed.
		ShibPS->UninitializeAbilitySystemForShibClass();
	}
}

#pragma region CustomMovementMode

void AShibCharacter::SetCustomMovementDisplay(ECustomMovementDisplay NewCustomMovementDisplay)
{
	if(HasAuthority() || IsLocallyControlled()) // Is replicated SkipOwner for other clients
	{
		CustomMovementDisplay = NewCustomMovementDisplay;
	}
}

TEnumAsByte<ECustomMovementDisplay> AShibCharacter::GetCustomMovementDisplay() const
{
	return CustomMovementDisplay;
}

#pragma endregion CustomMovementMode

void AShibCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!Cast<AShibBaseController>(Controller)) return;

	auto* TaggedInputComponent = Cast<UShibTaggedInputComponent>(PlayerInputComponent);
	checkf(TaggedInputComponent, TEXT("Invalid Input Configuration. No Tagged Input Component."));

	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_Move, ETriggerEvent::Triggered, this, &AShibCharacter::InputMove);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_Look, ETriggerEvent::Triggered, this, &AShibCharacter::InputLook);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_JumpAction, ETriggerEvent::Triggered, this, &AShibCharacter::InputJump);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_PrimaryAbility, ETriggerEvent::Triggered, this, &AShibCharacter::InputPrimaryAbility);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_SecondaryAbility, ETriggerEvent::Triggered, this, &AShibCharacter::InputSecondaryAbility);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_PickupAbility, ETriggerEvent::Triggered, this, &AShibCharacter::InputPickupAbility);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_DriftAction, ETriggerEvent::Started, this, &AShibCharacter::InputStartDrift);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_DriftAction, ETriggerEvent::Completed, this, &AShibCharacter::InputStopDrift);
	TaggedInputComponent->BindActionByTag(InputConfig, TAG_Input_RespawnAction, ETriggerEvent::Triggered, this, &AShibCharacter::InputRespawn);
}

// ---- Getters ----
 
TObjectPtr<AShibPlayerState> AShibCharacter::GetShibPS()
{
	if (!ShibPS) ShibPS = GetPlayerState<AShibPlayerState>();
	return ShibPS;
}

TObjectPtr<UShibGameInstance> AShibCharacter::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}


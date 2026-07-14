// Copyright Shiba Inu Games LLC.

#include "Character/ShibMovementComponent.h"

#include "Character/ShibCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void UShibMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if(ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		OwnerCharacter = Char;
	}

	if (!ensureAlwaysMsgf(AccelerationCurve, TEXT("No Acceleration Curve."))) return;
}

float UShibMovementComponent::GetMaxAcceleration() const
{
	// Max Acceleration is used as a modifier changed by the acceleration attribute
	return MaxAcceleration * AccelerationCurve->GetFloatValue(Velocity.Size2D());
}

#pragma region Flags

bool UShibMovementComponent::FSavedMove_Sy::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
                                                         float MaxDelta) const
{
	FSavedMove_Sy* NewSyMove = static_cast<FSavedMove_Sy*>(NewMove.Get());

	if(Saved_bWantsToDrift != NewSyMove->Saved_bWantsToDrift)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UShibMovementComponent::FSavedMove_Sy::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToDrift = 0;
}

uint8 UShibMovementComponent::FSavedMove_Sy::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if(Saved_bWantsToDrift)
		Result |= FLAG_Custom_0;
	
	return Result;
}

void UShibMovementComponent::FSavedMove_Sy::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if(UShibMovementComponent* SyMovement = Cast<UShibMovementComponent>(C->GetCharacterMovement()))
	{
		Saved_bWantsToDrift = SyMovement->Safe_bWantsToDrift;
	}
}

void UShibMovementComponent::FSavedMove_Sy::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	if(UShibMovementComponent* SyMovement = Cast<UShibMovementComponent>(C->GetCharacterMovement()))
	{
		SyMovement->Safe_bWantsToDrift = Saved_bWantsToDrift;
	}
}

UShibMovementComponent::FNetworkPredictionData_Client_Sy::FNetworkPredictionData_Client_Sy(
	const UCharacterMovementComponent& ClientMovement)
:	Super(ClientMovement)
{
}

FSavedMovePtr UShibMovementComponent::FNetworkPredictionData_Client_Sy::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Sy());
}

FNetworkPredictionData_Client* UShibMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if(ClientPredictionData == nullptr)
	{
		if(UShibMovementComponent* SyMovementComponent = const_cast<UShibMovementComponent*>(this))
		{
			SyMovementComponent->ClientPredictionData = new FNetworkPredictionData_Client_Sy(*this);
			SyMovementComponent->NetworkMaxSmoothUpdateDistance = 92.f;
			SyMovementComponent->NetworkNoSmoothUpdateDistance = 140.f;
		}
	}

	return ClientPredictionData;
}

void UShibMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToDrift = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

#pragma endregion Flags

#pragma region CustomMove

void UShibMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	// Called on everyone but flags are always false for external clients
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	if(Safe_bWantsToDrift)
	{
		EnterDrift();
	}
	else
	{
		ExitDrift();
	}

#pragma region Debug

	// FString MovementModeString;
	//
	// switch (MovementMode)
	// {
	// case MOVE_Walking:
	// 	MovementModeString = TEXT("Walking");
	// 	break;
	// case MOVE_NavWalking:
	// 	MovementModeString = TEXT("NavWalking");
	// 	break;
	// case MOVE_Falling:
	// 	MovementModeString = TEXT("Falling");
	// 	break;
	// case MOVE_Swimming:
	// 	MovementModeString = TEXT("Swimming");
	// 	break;
	// case MOVE_Flying:
	// 	MovementModeString = TEXT("Flying");
	// 	break;
	// case MOVE_Custom:
	// 	MovementModeString = TEXT("Custom");
	// 	break;
	// case MOVE_None:
	// 	MovementModeString = TEXT("None");
	// 	break;
	// default:
	// 	MovementModeString = TEXT("Unknown");
	// 	break;
	// }
	//
	// if( ! Safe_bWantsToDrift && MovementModeString == "Custom")
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("ERROR: Movement Mode is CUSTOM but FLAG doesn't want to drift"));
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Movement Mode: %s"), *MovementModeString);
	// }

#pragma endregion Debug
}

void UShibMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	Super::SetMovementMode(NewMovementMode, NewCustomMode);

	if(AShibCharacter* ShibCharacter = Cast<AShibCharacter>(GetOwner()))
	{
		if(IsCustomMovementMode(CMOVE_Drift))
		{
			ShibCharacter->SetCustomMovementDisplay(CMD_Drifting);
			return;
		}
		
		ShibCharacter->SetCustomMovementDisplay(CMD_None);
	}
}

FHitResult UShibMovementComponent::CheckFloor(float Distance) const
{
	const float CapsuleHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Start = UpdatedComponent->GetComponentLocation() - FVector(0,0, CapsuleHalfHeight);
	FVector End = Start;
	End.Z -= Distance;
	// DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 1.0f);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	
	return HitResult;
}

void UShibMovementComponent::TryEnterDrift()
{
	// Only called on owner
	bWantsToDrift = true;
	EnterDrift();
}

void UShibMovementComponent::TryExitDrift()
{
	// Only called on owner
	bWantsToDrift = false;
	if(CanExitDrift())
	{
		ExitDrift();
	}
}

bool UShibMovementComponent::CanDrift() const
{
	return MovementMode == MOVE_Walking;
}

bool UShibMovementComponent::CanExitDrift() const
{
	return IsCustomMovementMode(CMOVE_Drift);
}

void UShibMovementComponent::EnterDrift()
{
	if(CanDrift())
	{
		PhysInitialSpeed = Velocity.Size2D();
		PhysDistance = 0.f;
		SetMovementMode(MOVE_Custom, CMOVE_Drift);
	}
}

void UShibMovementComponent::ExitDrift()
{
	if(CanExitDrift())
	{
		SetMovementMode(MOVE_Walking, CMOVE_None);
	}
}

void UShibMovementComponent::PhysDrift(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const FHitResult FloorResult = CheckFloor(120);
	if( ! FloorResult.bBlockingHit)
	{
		SetMovementMode(MOVE_Falling, CMOVE_None);
		return;
	}

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector ForwardDirection = 	UpdatedComponent->GetForwardVector();

	// Define a sideways drift direction (perpendicular to forward).
	const FVector DriftDirection = FVector::CrossProduct(ForwardDirection, FVector::UpVector).GetSafeNormal();

	// Make sure speed doesn't drop below 0
	PhysInitialSpeed = FMath::Max(PhysInitialSpeed -= PhysDistance, 0.f);
	
	// Apply drift velocity by blending forward direction with sideways drift.
	const FVector DriftVelocity = ForwardDirection * PhysInitialSpeed + DriftDirection * DriftFactor;

	// Interpolate the velocity towards the target drift velocity for smooth transition.
	Velocity = FMath::VInterpTo(Velocity, DriftVelocity, deltaTime, 2.f);

	const float DotProduct = FVector::DotProduct(FloorResult.Normal, ForwardDirection);
	if(DotProduct < 0.f)
	{
		// // Running uphill
	}
	else if(DotProduct > 0.01f && FloorResult.Distance > 10)
	{
		// Running downhill
		Velocity.Z = FMath::Min( -PhysInitialSpeed, -300);
	}
	else
	{
		// Flat surface
	}
	
	ProcessMovementAgainstSurface(deltaTime);

	// Tracks how far we travelled in drifting to slow down movement
	PhysDistance = FVector::Dist(OldLocation, UpdatedComponent->GetComponentLocation()) * DriftDecelerate;
}

void UShibMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch(CustomMovementMode)
	{
	case CMOVE_Drift:
		PhysDrift(DeltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Custom Movement Mode"));
	}
}

bool UShibMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UShibMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Safe_bWantsToDrift = bWantsToDrift;
}

#pragma endregion CustomMove

#pragma region MovementProcess

void UShibMovementComponent::ProcessMovementAgainstSurface(float deltaTime)
{
	// Copy paste from PhysFlying().
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		const FVector GravDir = FVector(0.f, 0.f, -1.f);
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = GravDir | VelDir;

		bool bSteppedUp = false;
		if ((FMath::Abs(Hit.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			float stepZ = UpdatedComponent->GetComponentLocation().Z;
			bSteppedUp = StepUp(GravDir, Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				OldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (OldLocation.Z - stepZ);
			}
		}

		if (!bSteppedUp)
		{
			// adjust and try again
			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}

	if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

void UShibMovementComponent::NewProcessMovement(float deltaTime)
{
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		// adjust and try again
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

#pragma endregion MovementProcess
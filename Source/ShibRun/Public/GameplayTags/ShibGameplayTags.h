// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

//Declare Native Input Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Move);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Fly);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Look);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Jump);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Run);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_PrimaryAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_SecondaryAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_PickupAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_JumpAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_DriftAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_RespawnAction);

//Declare Native abilities Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PrimaryAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SecondaryAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PickupAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_JumpAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DriftAbility);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_RespawnAbility);

//Declare Native effects Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Cooldown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Buff);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Buff_SpeedBoost);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Movement_Tumble);

//Declare Native attributes Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Attribute_ShibSpeed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Attribute_ShibAcceleration);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Attribute_ShibFriction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Attribute_ShibHealth);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Attribute_ShibAirControl);

//Declare Native modifiers Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Modifier_DamageType)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Modifier_AI_Speed_Increase)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Modifier_AI_Speed_Decrease)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Modifier_AI_Accel)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Modifier_AI_Friction)

//Define Native misc Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead);
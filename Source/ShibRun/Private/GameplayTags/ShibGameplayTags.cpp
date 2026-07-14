// Copyright Shiba Inu Games LLC.

#include "GameplayTags/ShibGameplayTags.h"

//Define native Input Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move, "Input.Move")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Fly, "Input.Fly")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Look, "Input.Look")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Jump, "Input.Jump")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Run, "Input.Run")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_PrimaryAbility, "Input.Ability.Primary")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_SecondaryAbility, "Input.Ability.Secondary")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_PickupAbility, "Input.Ability.Pickup")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_JumpAction, "Input.Ability.JumpAction")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_DriftAction, "Input.Ability.DriftAction")
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_RespawnAction, "Input.Ability.Respawn")

//Define Native abilities Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_PrimaryAbility, "Ability.Primary")
UE_DEFINE_GAMEPLAY_TAG(TAG_SecondaryAbility, "Ability.Secondary")
UE_DEFINE_GAMEPLAY_TAG(TAG_PickupAbility, "Ability.Pickup")
UE_DEFINE_GAMEPLAY_TAG(TAG_JumpAbility, "Ability.Movement.Jump")
UE_DEFINE_GAMEPLAY_TAG(TAG_DriftAbility, "Ability.Movement.Drift")
UE_DEFINE_GAMEPLAY_TAG(TAG_RespawnAbility, "Ability.Movement.Respawn")

//Define Native Effects Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Cooldown, "Effect.Cooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Buff, "Effect.Buff");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Buff_SpeedBoost, "Effect.Buff.SpeedBoost");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Movement_Tumble, "Effect.Movement.Tumble");

//Define Native attributes Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Attribute_ShibSpeed, "Attribute.Char.Movement.ShibSpeed")
UE_DEFINE_GAMEPLAY_TAG(TAG_Attribute_ShibAcceleration, "Attribute.Char.Movement.ShibAcceleration")
UE_DEFINE_GAMEPLAY_TAG(TAG_Attribute_ShibFriction, "Attribute.Char.Movement.ShibFriction")
UE_DEFINE_GAMEPLAY_TAG(TAG_Attribute_ShibHealth, "Attribute.Char.Life.ShibHealth")
UE_DEFINE_GAMEPLAY_TAG(TAG_Attribute_ShibAirControl, "Attribute.Char.Movement.ShibAirControl")

//Define Native modifiers Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Modifier_DamageType, "Modifier.DamageType")
UE_DEFINE_GAMEPLAY_TAG(TAG_Modifier_AI_Speed_Increase, "Modifier.AI.Speed.Increase")
UE_DEFINE_GAMEPLAY_TAG(TAG_Modifier_AI_Speed_Decrease, "Modifier.AI.Speed.Decrease")
UE_DEFINE_GAMEPLAY_TAG(TAG_Modifier_AI_Accel, "Modifier.AI.Accel")
UE_DEFINE_GAMEPLAY_TAG(TAG_Modifier_AI_Friction, "Modifier.AI.Friction")

//Define Native misc Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death, "Status.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death_Dying, "Status.Death.Dying");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death_Dead, "Status.Death.Dead");
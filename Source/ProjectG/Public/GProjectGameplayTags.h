// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace GProjectGameplayTags
{
	//InputTag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Movement_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Movement_Jump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Movement_Dash);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Combat_BasicAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Combat_StrongAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Combat_DashAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Combat_WeaponAttack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Skill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Skill_Accessory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Targeting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Targeting_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Camera);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Camera_Look);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction_UseItem);
	//State
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Invulnerable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Hitstun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Knockdown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Targeting_LockOn);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_Knockdown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Accessory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Targeting_LockOn);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Root);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Combat_BasicAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Combat_StrongAttack);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Combo_Window_Open);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Combo_Window_Close);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Attack_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Knockdown);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Combat_Damage);
}

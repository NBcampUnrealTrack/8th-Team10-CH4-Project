// Copyright Epic Games, Inc. All Rights Reserved.

#include "GProjectGameplayTags.h"

namespace GProjectGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Movement, "InputTag.Movement");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Movement_Move, "InputTag.Movement.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Movement_Jump, "InputTag.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Movement_Dash, "InputTag.Movement.Dash");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Combat, "InputTag.Combat");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Combat_BasicAttack, "InputTag.Combat.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Combat_StrongAttack, "InputTag.Combat.StrongAttack");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Combat_DashAttack, "InputTag.Combat.DashAttack");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Combat_WeaponAttack, "InputTag.Combat.WeaponAttack");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill, "InputTag.Skill");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Accessory, "InputTag.Skill.Accessory");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Targeting, "InputTag.Targeting");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Targeting_LockOn, "InputTag.Targeting.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Camera, "InputTag.Camera");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Camera_Look, "InputTag.Camera.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interaction, "InputTag.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interaction_Interact, "InputTag.Interaction.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interaction_UseItem, "InputTag.Interaction.UseItem");

	UE_DEFINE_GAMEPLAY_TAG(State_Character_Dead, "State.Character.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Invulnerable, "State.Character.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Hitstun, "State.Combat.Hitstun");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Knockdown, "State.Combat.Knockdown");
	UE_DEFINE_GAMEPLAY_TAG(State_Targeting_LockOn, "State.Targeting.LockOn");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_Attack, "Ability.Combat.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_HitReact, "Ability.Combat.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_Knockdown, "Ability.Combat.Knockdown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Dash, "Ability.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Accessory, "Ability.Skill.Accessory");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Targeting_LockOn, "Ability.Targeting.LockOn");

	UE_DEFINE_GAMEPLAY_TAG(Event_Root, "Event");
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Combat_BasicAttack, "Event.Input.Combat.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Combat_StrongAttack, "Event.Input.Combat.StrongAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Combo_Window_Open, "Event.Combat.Combo.Window.Open");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Combo_Window_Close, "Event.Combat.Combo.Window.Close");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Attack_Hit, "Event.Combat.Attack.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact, "Event.Combat.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Knockdown, "Event.Combat.Knockdown");

	UE_DEFINE_GAMEPLAY_TAG(Data_Combat_Damage, "Data.Combat.Damage");
}

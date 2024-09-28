// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponCarriedAmmo : uint8
{
	EWCA_Default UMETA(DisplayName = "Default"),
	EWCA_AssaultRifleCarriedAmmo = 60 UMETA(DisplayName = "Assault Rifle Carried Ammo") ,

	EWCA_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType) 
enum class ETeamID : uint8

{
	Spectator = 0 UMETA(DisplayName = "Spectator"),
	Red = 1 UMETA(DisplayName = "Red"),
	Blue = 2 UMETA(DisplayName = "Blue"),
	Green = 3 UMETA(DisplayName = "Green"),
	Yello = 4 UMETA(DisplayName = "Yello"),
	NoTeamId = 255 UMETA(DisplayName = "NoTeamId")
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_OnFoot UMETA(DisplayName = "OnFoot"),
	ECS_Riding UMETA(DisplayName = "Riding"),
	ECS_Mounting UMETA(DisplayName = "Mounting"),
	ECS_Sprinting UMETA(DisplayName = "Sprinting"),
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
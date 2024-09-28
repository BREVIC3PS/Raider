// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "../Weapon/Weapon.h"
#include "../NewShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "../PlayerController/TPSPlayerController.h"
#include "../HUD/CharacterHUD.h"
#include <NewShooter/Weapon/ProjectileWeapon_Base.h>

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (Character->HasAuthority())
		{
			//服务端初始化携带弹药量，此时不同步给客户端，直到客户端捡起武器再同步
			InitializeAmmo();
		}

	}
	Controller = Controller == nullptr ? Cast<ATPSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ACharacterHUD>(Controller->GetHUD()) : HUD;
	}

}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//客户端调用RPC。服务端调用EquipWeapon()之后，改变EquippedWeapon的状态，在客户端调用RepNotify函数OnRep_EquippedWeapon()如下，实现武器的装备
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->Equipped(Character);
		SetCharacterEquipped();
		Controller = Controller == nullptr ? Cast<ATPSPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDCrosshair();
		}
		//HUD变化将在OnRep_CurrentCarriedAmmo()中更新，此处不再更新
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}
//本地执行
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		if (EquippedWeapon == nullptr) return;
		if (Character)
		{
			Character->PlayFireMontage(bAiming);
			EquippedWeapon->Fire(HitTarget);
		}

		ServerFire(HitTarget);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = .75f;
		}
		StartFireTimer();
	}
	
}

//客户端调用服务端RPC
void UCombatComponent::Reload()
{
	if (CurrentCarriedAmmo && EquippedWeapon && EquippedWeapon->Ammo != EquippedWeapon->MagCapacity)
	{
		ServerReload();
	}
	
}

//AnimNotify本地调用
void UCombatComponent::ReloadComplete()
{
	ServerReloadComplete(); //告诉Server已换弹完毕，更新子弹吧
	
}

//服务器更新子弹，改变Ammo的值，自动Replicate到客户端调用RepNotify调用SetHUD
void UCombatComponent::ServerReloadComplete_Implementation()
{
	if (EquippedWeapon)
	{
		// 计算需要补充的子弹数量
		int ammoNeeded = EquippedWeapon->MagCapacity - EquippedWeapon->Ammo;

		// 如果需要补充的子弹数量大于0
		if (ammoNeeded > 0)
		{
			// 如果拥有的备弹足够补充
			if (CurrentCarriedAmmo >= ammoNeeded)
			{
				// 从拥有的备弹中减去需要补充的子弹数量
				CurrentCarriedAmmo -= ammoNeeded;
				// 弹匣中的子弹数量增加
				EquippedWeapon->Ammo += ammoNeeded;
			}
			else
			{
				// 如果拥有的备弹不足以补充弹匣，将所有备弹都装入弹匣
				EquippedWeapon->Ammo += CurrentCarriedAmmo;
				// 将拥有的备弹数量设为0
				CurrentCarriedAmmo = 0;
			}
		}
		CarriedAmmoMap[EquippedWeapon->WeaponType] = CurrentCarriedAmmo;
	}
	CombatState = ECombatState::ECS_Unoccupied;
}

//服务端Reload
void UCombatComponent::ServerReload_Implementation()
{
	if (Character)
	{
		CombatState = ECombatState::ECS_Reloading; //此时调用RPC:OnRep_CombatState播放动画，子弹在Anim_Notify:ReloadComplete之后更新
	}
}

//本地执行
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (!HUD)return;

	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 90.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit) {
			TraceHitResult.ImpactPoint = End;
		}
	}
}

//本地执行
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || !Character->IsLocallyControlled()) return;

	Controller = Controller == nullptr ? Cast<ATPSPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ACharacterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUD->CrosshairsCenter = CrosshairsCenter;
			}
			else
			{
				HUD->CrosshairsCenter = nullptr;
			}
		}
	}
}

//服务端执行
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//服务端执行
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

//服务端+客户端执行
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled())return;
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character) return;
	EquippedWeapon->bCanFire = false;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	EquippedWeapon->bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (CombatState != ECombatState::ECS_Unoccupied)return false;
	return (!EquippedWeapon->IsEmpty() || !EquippedWeapon->bCanFire);
}

//客户端执行
void UCombatComponent::OnRep_CurrentCarriedAmmo()
{
	if(Controller)
	Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
}

void UCombatComponent::InitializeAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, static_cast<int32>(EWeaponCarriedAmmo::EWCA_AssaultRifleCarriedAmmo));
}

void UCombatComponent::OnRep_CombatState()
{
	if (Character && CombatState==ECombatState::ECS_Reloading)
	{
		Character->PlayReloadMontage();
		if (EquippedWeapon)
		{
			EquippedWeapon->PlayReloadAnimation();
		}
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//客户端执行
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
		//SetHUDCrosshairs(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CurrentCarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

//在服务端调用
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	//如果已装备有同样类型的武器,直接增加子弹即可
	if (EquippedWeapon && WeaponToEquip->WeaponType == EquippedWeapon->WeaponType)
	{
		CarriedAmmoMap[EquippedWeapon->WeaponType] += WeaponToEquip->Ammo;
		CurrentCarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
		WeaponToEquip->Destroy();
		return;
	}
	EquippedWeapon = WeaponToEquip; //调用RPC：OnRep_EquippedWeapon()
	EquippedWeapon->Equipped(Character);
	SetCharacterEquipped();
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		CurrentCarriedAmmo += CarriedAmmoMap[EquippedWeapon->WeaponType];//调用RPC，同步CurrentCarriedAmmo变化
	}

}


void UCombatComponent::SetCharacterEquipped()
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	Character->bUseControllerRotationPitch = false;
	Character->bUseControllerRotationRoll = false;
	Character->GetFollowCamera()->SetRelativeLocation(EquippedWeapon->CameraOffset);
	Character->GetFollowCamera()->SetRelativeRotation(EquippedWeapon->CameraRotation);
}


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
			//����˳�ʼ��Я����ҩ������ʱ��ͬ�����ͻ��ˣ�ֱ���ͻ��˼���������ͬ��
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

//�ͻ��˵���RPC������˵���EquipWeapon()֮�󣬸ı�EquippedWeapon��״̬���ڿͻ��˵���RepNotify����OnRep_EquippedWeapon()���£�ʵ��������װ��
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
		//HUD�仯����OnRep_CurrentCarriedAmmo()�и��£��˴����ٸ���
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
//����ִ��
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

//�ͻ��˵��÷����RPC
void UCombatComponent::Reload()
{
	if (CurrentCarriedAmmo && EquippedWeapon && EquippedWeapon->Ammo != EquippedWeapon->MagCapacity)
	{
		ServerReload();
	}
	
}

//AnimNotify���ص���
void UCombatComponent::ReloadComplete()
{
	ServerReloadComplete(); //����Server�ѻ�����ϣ������ӵ���
	
}

//�����������ӵ����ı�Ammo��ֵ���Զ�Replicate���ͻ��˵���RepNotify����SetHUD
void UCombatComponent::ServerReloadComplete_Implementation()
{
	if (EquippedWeapon)
	{
		// ������Ҫ������ӵ�����
		int ammoNeeded = EquippedWeapon->MagCapacity - EquippedWeapon->Ammo;

		// �����Ҫ������ӵ���������0
		if (ammoNeeded > 0)
		{
			// ���ӵ�еı����㹻����
			if (CurrentCarriedAmmo >= ammoNeeded)
			{
				// ��ӵ�еı����м�ȥ��Ҫ������ӵ�����
				CurrentCarriedAmmo -= ammoNeeded;
				// ��ϻ�е��ӵ���������
				EquippedWeapon->Ammo += ammoNeeded;
			}
			else
			{
				// ���ӵ�еı��������Բ��䵯ϻ�������б�����װ�뵯ϻ
				EquippedWeapon->Ammo += CurrentCarriedAmmo;
				// ��ӵ�еı���������Ϊ0
				CurrentCarriedAmmo = 0;
			}
		}
		CarriedAmmoMap[EquippedWeapon->WeaponType] = CurrentCarriedAmmo;
	}
	CombatState = ECombatState::ECS_Unoccupied;
}

//�����Reload
void UCombatComponent::ServerReload_Implementation()
{
	if (Character)
	{
		CombatState = ECombatState::ECS_Reloading; //��ʱ����RPC:OnRep_CombatState���Ŷ������ӵ���Anim_Notify:ReloadComplete֮�����
	}
}

//����ִ��
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

//����ִ��
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

//�����ִ��
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//�����ִ��
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

//�����+�ͻ���ִ��
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

//�ͻ���ִ��
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
	
	//�ͻ���ִ��
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

//�ڷ���˵���
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	//�����װ����ͬ�����͵�����,ֱ�������ӵ�����
	if (EquippedWeapon && WeaponToEquip->WeaponType == EquippedWeapon->WeaponType)
	{
		CarriedAmmoMap[EquippedWeapon->WeaponType] += WeaponToEquip->Ammo;
		CurrentCarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
		WeaponToEquip->Destroy();
		return;
	}
	EquippedWeapon = WeaponToEquip; //����RPC��OnRep_EquippedWeapon()
	EquippedWeapon->Equipped(Character);
	SetCharacterEquipped();
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		CurrentCarriedAmmo += CarriedAmmoMap[EquippedWeapon->WeaponType];//����RPC��ͬ��CurrentCarriedAmmo�仯
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


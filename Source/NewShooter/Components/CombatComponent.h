// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NewShooter.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEWSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	// Sets default values for this component's properties
	friend class ANewShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SetCharacterEquipped();

	UPROPERTY()
	class ACharacterHUD* HUD;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void Reload();

	UFUNCTION(BlueprintCallable)
	void ReloadComplete();

	UFUNCTION(Server, Reliable)
	void ServerReloadComplete();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);


private:
	UPROPERTY()
	class ANewShooterCharacter* Character;
	UPROPERTY()
	class ATPSPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	* HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;


	/**
	* Aiming and FOV
	*/

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	void InterpFOV(float DeltaTime);

	FTimerHandle FireTimer;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CurrentCarriedAmmo)
	int32 CurrentCarriedAmmo;

	UFUNCTION()
	void OnRep_CurrentCarriedAmmo();

	UPROPERTY(VisibleAnywhere)
	TMap<EWeaponType, int32> CarriedAmmoMap;

	void InitializeAmmo();

	UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

};

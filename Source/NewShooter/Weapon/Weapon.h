// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NewShooter.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class NEWSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	

	/**
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 60.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	

	bool bCanFire = true;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
private:
	UPROPERTY()
	class ANewShooterCharacter* OwnerCharacter;
	UPROPERTY()
	class ATPSPlayerController* OwnerController;


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();


	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* ReloadAnimation;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	void SetHUDAmmo();

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<class ACasing> CasingClass;
public:
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	void SetWeaponState(EWeaponState State);
	void SetMeshCollision(bool In);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	UPROPERTY(EditAnywhere, Category = Camera)
	FVector CameraOffset = FVector(0.0f, 100.f, 100.0f);
	UPROPERTY(EditAnywhere, Category = Camera)
	FRotator CameraRotation = FRotator(FRotator(0,0,0));

	void Equipped(ANewShooterCharacter* OwningCharacter);

	void Dropped();

	bool IsEmpty();

	void PlayReloadAnimation();

};

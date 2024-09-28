// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon_Base.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API AProjectileWeapon_Base : public AWeapon
{
	GENERATED_BODY()
	
protected:
public:
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector& HitTarget) override;

	FVector GetMuzzelLocation();

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
};

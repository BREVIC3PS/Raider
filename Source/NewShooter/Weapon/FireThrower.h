// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "FireThrower.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API AFireThrower : public AWeapon
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere)
	float FireRange = 1600.f;

	UPROPERTY(EditAnywhere)
	float DamageAmount = 10.f;

	AFireThrower();

	FRotator Interp_Rotation;
	FRotator TargetFireRotation;
	
protected:
	virtual void BeginPlay() override;
	// 喷火特效组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* FlamethrowerEffect;

	// 喷火特效资源
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* FlamethrowerEffectAsset;

	// 用于调整火焰特效位置的场景组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	USceneComponent* FlamethrowerEffectLocation;
public:
	virtual void Tick(float DeltaTime) override;
	bool Activated = false;
	FTimerHandle FireTimer;
	void FireTimerFinished();
	void StartFireTimer();
	FORCEINLINE float GetFireRange() const { return FireRange; }
	class ADragon* OwnerDragon;
	void Fire(const FVector& HitTarget);
	void StopFire();
	
	virtual void SetOwner(AActor* NewOwner) override;

	UFUNCTION(Server, Unreliable)
	void ServerFire(const FVector& HitTarget);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFire(const FRotator& ActorRotation, const FRotator& TargetRotation);

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopFire();
	void CapsuleTrace(const FVector& HitTarget, bool& bHit, TArray<FHitResult>& HitResults);
	void CalculateTraceStartAndEndLocation(const FVector& HitTarget, FVector& StartLocation, FVector& EndLocation);
};

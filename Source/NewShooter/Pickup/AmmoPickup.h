// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API AAmmoPickup : public APickupBase
{
	GENERATED_BODY()
	
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	) override;
	
	UPROPERTY(EditAnywhere)
	int32 AmmoCount;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAddAmmo(ANewShooterCharacter* CharacterToAttach);

public:
	AAmmoPickup();

	UFUNCTION(BlueprintImplementableEvent)
	void SphereOverlap(AActor* OtherActor);
	
};

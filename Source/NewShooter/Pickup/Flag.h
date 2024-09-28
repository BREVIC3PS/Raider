// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API AFlag : public APickupBase
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UHealthBarWidgetComponent* HealthBarWidget;

	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void StartCountDown();
	void StopCountDown();
	FTimerHandle PickupTimer;

	UFUNCTION()
	void OnRep_StartCountdown();

	UPROPERTY(ReplicatedUsing = OnRep_StartCountdown)
	bool bStartCountdown = false;

	UPROPERTY(EditAnywhere)
	float CaptureSpeed = 0.2;

	float Progress = 0;

public:
	AFlag();
	void Dropped();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttach(ANewShooterCharacter* CharacterToAttach);
};

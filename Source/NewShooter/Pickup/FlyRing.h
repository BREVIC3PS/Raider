// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.h"
#include "FlyRing.generated.h"

UCLASS()
class NEWSHOOTER_API AFlyRing : public APickupBase
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category = "Properties")
	UStaticMeshComponent* StaticMesh;

public:	
	// Sets default values for this actor's properties
	AFlyRing();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* ActivateEffect;

	// 喷火特效资源
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ActivateEffectAsset;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

	virtual void OnRep_Activated() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ReActivate() override;
};

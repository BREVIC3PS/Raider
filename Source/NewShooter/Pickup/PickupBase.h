// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class NEWSHOOTER_API APickupBase : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Properties")
	USkeletalMeshComponent* SkeletalMesh;

public:	
	// Sets default values for this actor's properties
	APickupBase();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* Effect;

	// 喷火特效资源
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* EffectAsset;

	// 用于调整火焰特效位置的场景组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	USceneComponent* EffectLocation;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	int32 ActorOverlapped = 0;

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
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	virtual void OnRep_Activated();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Activated)
	bool bActivated = true;

	UPROPERTY()
	FTimerHandle RefreshTimer;

	UPROPERTY(EditAnywhere)
	float RefreshDelay;
public:	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class ANewShooterCharacter* OverlappCharacter;
	
	FORCEINLINE USkeletalMeshComponent* GetMesh() { return SkeletalMesh; }

	virtual void ReActivate();
};

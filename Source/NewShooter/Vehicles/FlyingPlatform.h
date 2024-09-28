// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlyingPlatform.generated.h"

UCLASS()
class NEWSHOOTER_API AFlyingPlatform : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFlyingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*UPROPERTY(VisibleAnywhere, Category = "Properties")
	USkeletalMeshComponent* SKPlatformMesh;*/

	UPROPERTY(VisibleAnywhere, Category = "Properties")
	UStaticMeshComponent* SMPlatformMesh;

	/*UPROPERTY(VisibleAnywhere, Category = "Properties")
	class USphereComponent* AreaSphere;*/
	
};

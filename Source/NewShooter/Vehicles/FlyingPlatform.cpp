// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingPlatform.h"


// Sets default values
AFlyingPlatform::AFlyingPlatform()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SMPlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(SMPlatformMesh);
}

// Called when the game starts or when spawned
void AFlyingPlatform::BeginPlay()
{
	Super::BeginPlay();
	bReplicates = true;
	SetReplicateMovement(true);
}

// Called every frame
void AFlyingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFlyingPlatform::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


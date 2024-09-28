// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyRing.h"
#include "NewShooter/NewShooterCharacter.h"
#include "Components/SphereComponent.h"
#include <Net/UnrealNetwork.h>
#include <NewShooter/GameMode/TPSGameMode.h>
#include "Particles/ParticleSystemComponent.h"
#include "../PlayerController/TPSPlayerController.h"

// Sets default values
AFlyRing::AFlyRing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	/*StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	if (StaticMesh)
	{
		SetRootComponent(StaticMesh);
		StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}*/

	// 创建粒子系统组件并附加到场景组件
	ActivateEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ActiveEffect"));
	ActivateEffect->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AFlyRing::BeginPlay()
{
	Super::BeginPlay();
	Effect->Activate();
	if (ActivateEffect)
	{
		ActivateEffect->SetTemplate(ActivateEffectAsset);
	}
	if (ActivateEffect)ActivateEffect->Deactivate();
}

void AFlyRing::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bActivated)return;
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OverlappCharacter && OverlappCharacter->CarringFlag)
	{
		ATPSPlayerController* Controller = OverlappCharacter->GetSavedTPSPlayerController();
		if (Controller)
		{
			ATPSGameMode* GameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
			if (GameMode)//Server
			{
				GameMode->PlayerWalkThroughRing(Controller);
				bActivated = false;//Replicated
			}
		}
	}
}

void AFlyRing::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bActivated)return;
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AFlyRing::OnRep_Activated()
{
	Super::OnRep_Activated();
	if (!ActivateEffect)return;
	if (bActivated)
	{
		ActivateEffect->Deactivate();
	}
	else
	{
		ActivateEffect->Activate();
	}
}


void AFlyRing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFlyRing::ReActivate()
{
	ActivateEffect->Deactivate();
	bActivated = true;
	OverlappCharacter = nullptr;
	ActorOverlapped = 0;
}


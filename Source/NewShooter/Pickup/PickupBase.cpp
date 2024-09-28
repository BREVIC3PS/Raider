// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "NewShooter/NewShooterCharacter.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	if (SkeletalMesh)
	{
		SetRootComponent(SkeletalMesh);

		SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	}
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建场景组件并附加到角色的根组件
	EffectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("EffectLocation"));
	EffectLocation->SetupAttachment(RootComponent);

	// 创建粒子系统组件并附加到场景组件
	Effect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Effect"));
	Effect->SetupAttachment(EffectLocation);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
	}
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &APickupBase::OnSphereEndOverlap);

	if (EffectAsset)
	{
		Effect->SetTemplate(EffectAsset);
	}
	if(Effect)Effect->Activate();
	
}

void APickupBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ANewShooterCharacter* Character = Cast<ANewShooterCharacter>(OtherActor);
	if (Character)
	{
		OverlappCharacter = Character;
		ActorOverlapped++;
	}
}

void APickupBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ANewShooterCharacter* Character = Cast<ANewShooterCharacter>(OtherActor);
	if (Character)
	{
		OverlappCharacter = nullptr;
		ActorOverlapped--;
	}
}

void APickupBase::OnRep_Activated()
{
}

void APickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickupBase, bActivated);
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupBase::ReActivate()
{
	bActivated = true;
}


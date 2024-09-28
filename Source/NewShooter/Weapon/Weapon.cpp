// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "../NewShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "../PlayerController/TPSPlayerController.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::OnRep_WeaponState()
{
	SetWeaponState(WeaponState);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
}
//服务端调用
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ANewShooterCharacter* NewShooterCharacter = Cast<ANewShooterCharacter>(OtherActor);
	if (NewShooterCharacter)
	{
		NewShooterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ANewShooterCharacter* NewShooterCharacter = Cast<ANewShooterCharacter>(OtherActor);
	if (NewShooterCharacter)
	{
		NewShooterCharacter->SetOverlappingWeapon(nullptr);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

//被Multicast到所有客户端执行
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	SpendRound();
	/*if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}*/
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (!Owner)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<ANewShooterCharacter>(GetOwner()) : OwnerCharacter;
		if (OwnerCharacter)
		{
			OwnerController = OwnerController == nullptr ? Cast<ATPSPlayerController>(OwnerCharacter->Controller) : OwnerController;
		}
		SetHUDAmmo();
	}
}

void AWeapon::SetHUDAmmo()
{
	if (!OwnerCharacter || !OwnerController)return;
	OwnerController->SetHUDAmmo(Ammo);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		if (HasAuthority())AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetMeshCollision(false);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		SetMeshCollision(true);
		break;
	}
}

void AWeapon::SetMeshCollision(bool In)
{
	WeaponMesh->SetSimulatePhysics(In);
	WeaponMesh->SetEnableGravity(In);
	if (In)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		WeaponMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	}

	else
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//客户端调用
void AWeapon::Equipped(ANewShooterCharacter *OwningCharacter)
{
	SetWeaponState(EWeaponState::EWS_Equipped);
	SetOwner(OwningCharacter);
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ANewShooterCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		const USkeletalMeshSocket* HandSocket = OwnerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(this, OwnerCharacter->GetMesh());
		}
		OwnerController = OwnerController == nullptr ? Cast<ATPSPlayerController>(OwnerCharacter->Controller) : OwnerController;
	}
	SetHUDAmmo();
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;;
}

void AWeapon::PlayReloadAnimation()
{
	if (ReloadAnimation)
	{
		WeaponMesh->PlayAnimation(ReloadAnimation, false);
	}
}


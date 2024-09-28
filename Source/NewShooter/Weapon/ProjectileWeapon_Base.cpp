// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon_Base.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//Multicast到所有端执行
void AProjectileWeapon_Base::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//if (!HasAuthority()) return;

	//UE_LOG(LogTemp, Warning, TEXT("Name: %s, Owner:%s"), *GetName(), *GetOwner()->GetName());
	if (GetNetMode() == ENetMode::NM_DedicatedServer)return;
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
				);
			}
		}
	}
}

FVector AProjectileWeapon_Base::GetMuzzelLocation()
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		return SocketTransform.GetLocation();
	}
	else return FVector();
}

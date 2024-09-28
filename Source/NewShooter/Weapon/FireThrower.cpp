// Fill out your copyright notice in the Description page of Project Settings.


#include "FireThrower.h"
#include "Engine/SkeletalMeshSocket.h"
#include <Kismet/KismetSystemLibrary.h>
#include "../Dragon/Dragon.h"
#include "../NewShooterCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "Particles/ParticleSystemComponent.h"
#include <Kismet/KismetMathLibrary.h>

AFireThrower::AFireThrower()
{
	// 创建场景组件并附加到角色的根组件
	FlamethrowerEffectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("FlamethrowerEffectLocation"));
	FlamethrowerEffectLocation->SetupAttachment(RootComponent);

	// 创建粒子系统组件并附加到场景组件
	FlamethrowerEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FlamethrowerEffect"));
	FlamethrowerEffect->SetupAttachment(FlamethrowerEffectLocation);

	FireDelay = 0.1f;
}

void AFireThrower::BeginPlay()
{
	Super::Super::BeginPlay();
	// 设置粒子系统组件的模板为喷火特效资源
	if (FlamethrowerEffectAsset)
	{
		FlamethrowerEffect->SetTemplate(FlamethrowerEffectAsset);
	}
	FlamethrowerEffect->Deactivate();
	Interp_Rotation = GetActorRotation();
}

void AFireThrower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Activated && OwnerDragon && OwnerDragon->GetLocalRole() == ENetRole::ROLE_SimulatedProxy )
	{
		Interp_Rotation = UKismetMathLibrary::RInterpTo(Interp_Rotation, TargetFireRotation, DeltaTime, 8);
		SetActorRotation(Interp_Rotation);
	}
}

void AFireThrower::FireTimerFinished()
{
	bCanFire = true;
	if (OwnerDragon->bFireButtonPressed && bAutomatic)
	{
		OwnerDragon->TraceAndFire(this);
	}
}

void AFireThrower::StartFireTimer()
{
	bCanFire = false;
	GetWorldTimerManager().SetTimer(FireTimer, this, &AFireThrower::FireTimerFinished, FireDelay, false);
}

void AFireThrower::Fire(const FVector& HitTarget)
{
	if (!Activated && bCanFire)
	{
		// 激活喷火特效
		FlamethrowerEffect->Activate();
		Activated = true;
	}
	//DrawDebugSphere(GetWorld(), HitTarget, 50, 5, FColor::Green, false, 3);
	SetActorRotation((HitTarget - GetActorLocation()).Rotation());
	if (bCanFire)
	{
		ServerFire(HitTarget);
		StartFireTimer();
	}
	
	
}

void AFireThrower::StopFire()
{

	FlamethrowerEffect->Deactivate();
	Activated = false;
	
	// 停止喷火特效
	ServerStopFire();
}

void AFireThrower::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	OwnerDragon = Cast<ADragon>(NewOwner);
}

void AFireThrower::MulticastFire_Implementation(const FRotator& ActorRotation, const FRotator& TargetRotation)
{
	if (!Activated)
	{
		// 激活喷火特效
		FlamethrowerEffect->Activate();
		Activated = true;
	}
	TargetFireRotation = TargetRotation;
}

void AFireThrower::ServerFire_Implementation(const FVector& HitTarget)
{
	FRotator TargetRotation = ((HitTarget - GetActorLocation()).Rotation());
	SetActorRotation(TargetRotation);
	MulticastFire(GetActorRotation(), TargetRotation);

	// From muzzle flash socket to hit location from TraceUnderCrosshairs
	bool bHit = false;
	TArray<FHitResult> HitResults; // 用于存储碰撞信息的数组
	CapsuleTrace(HitTarget, bHit, HitResults);
	// 如果有碰撞，遍历所有碰撞的对象，并应用伤害
	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				ANewShooterCharacter* HitCharacter = Cast<ANewShooterCharacter>(HitActor);
				if(HitCharacter)
				{
					DrawDebugSphere(GetWorld(), HitActor->GetActorLocation(), 50, 10, FColor::Red, false, 2);
					//UE_LOG(LogTemp, Warning, TEXT("Hit Character:%s"), *HitCharacter->GetName());
					UGameplayStatics::ApplyDamage(HitActor, DamageAmount, OwnerDragon->Controller, this, UDamageType::StaticClass());
				}
			}
		}
	}

}

void AFireThrower::ServerStopFire_Implementation()
{
	MulticastStopFire();
}

void AFireThrower::MulticastStopFire_Implementation()
{
	FlamethrowerEffect->Deactivate();
	Activated = false;
}


void AFireThrower::CapsuleTrace(const FVector& HitTarget, bool& bHit, TArray<FHitResult>& HitResults)
{

	FVector StartLocation, EndLocation;
	CalculateTraceStartAndEndLocation(HitTarget, StartLocation, EndLocation);

	float Radius = 100.0f; // 胶囊的半径，可以根据需要调整
	float HalfHeight = 500.0f; // 胶囊的半高，可以根据需要调整

	// 创建胶囊形状
	//FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	// 创建碰撞查询参数
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 忽略自身
	QueryParams.AddIgnoredActor(GetOwner()); // 忽略自身
	QueryParams.AddIgnoredActor(OwnerDragon->PrimaryRidingCharacter);
	// 执行胶囊形状的碰撞检测
	bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity, // 旋转，这里我们使用默认的无旋转
		ECollisionChannel::ECC_Visibility, // 碰撞通道，可以根据需要选择不同的通道
		SphereShape,
		QueryParams
	);
	/*DrawDebugSphere(GetWorld(), StartLocation, Radius, 5, FColor::Green, false, 3);
	DrawDebugSphere(GetWorld(), (StartLocation + EndLocation) / 2, Radius, 5, FColor::Green, false, 3);
	DrawDebugSphere(GetWorld(), EndLocation, Radius, 5, FColor::Blue, false, 3);*/
}

void AFireThrower::CalculateTraceStartAndEndLocation(const FVector& HitTarget, FVector& StartLocation, FVector& EndLocation)
{
	FVector ToTarget = HitTarget - GetActorLocation();
	ToTarget.Normalize();
	StartLocation = GetActorLocation();
	EndLocation = StartLocation + ToTarget * FireRange;
}

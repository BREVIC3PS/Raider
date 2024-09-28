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
	// ����������������ӵ���ɫ�ĸ����
	FlamethrowerEffectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("FlamethrowerEffectLocation"));
	FlamethrowerEffectLocation->SetupAttachment(RootComponent);

	// ��������ϵͳ��������ӵ��������
	FlamethrowerEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FlamethrowerEffect"));
	FlamethrowerEffect->SetupAttachment(FlamethrowerEffectLocation);

	FireDelay = 0.1f;
}

void AFireThrower::BeginPlay()
{
	Super::Super::BeginPlay();
	// ��������ϵͳ�����ģ��Ϊ�����Ч��Դ
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
		// ���������Ч
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
	
	// ֹͣ�����Ч
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
		// ���������Ч
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
	TArray<FHitResult> HitResults; // ���ڴ洢��ײ��Ϣ������
	CapsuleTrace(HitTarget, bHit, HitResults);
	// �������ײ������������ײ�Ķ��󣬲�Ӧ���˺�
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

	float Radius = 100.0f; // ���ҵİ뾶�����Ը�����Ҫ����
	float HalfHeight = 500.0f; // ���ҵİ�ߣ����Ը�����Ҫ����

	// ����������״
	//FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	// ������ײ��ѯ����
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // ��������
	QueryParams.AddIgnoredActor(GetOwner()); // ��������
	QueryParams.AddIgnoredActor(OwnerDragon->PrimaryRidingCharacter);
	// ִ�н�����״����ײ���
	bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity, // ��ת����������ʹ��Ĭ�ϵ�����ת
		ECollisionChannel::ECC_Visibility, // ��ײͨ�������Ը�����Ҫѡ��ͬ��ͨ��
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

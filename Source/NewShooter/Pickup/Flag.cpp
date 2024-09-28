// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "NewShooter/Widgets/HealthBarWidgetComponent.h"
#include "NewShooter/NewShooterCharacter.h"
#include "Components/SphereComponent.h"
#include <Net/UnrealNetwork.h>
#include <NewShooter/GameMode/TPSGameMode.h>

void AFlag::MulticastAttach_Implementation(ANewShooterCharacter *CharacterToAttach)
{
	bActivated = false;
	GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	ANewShooterCharacter* OwnerCharacter = Cast<ANewShooterCharacter>(GetOwner());
	if (OwnerCharacter)OwnerCharacter->CarringFlag = nullptr;
	CharacterToAttach->CarringFlag = this;
	SetOwner(CharacterToAttach);
	StopCountDown();
	bool Attached = GetMesh()->AttachToComponent(CharacterToAttach->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("FlagSocket"));
}

AFlag::AFlag()
{
	HealthBarWidget = CreateDefaultSubobject<UHealthBarWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
}

void AFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bActivated)return;
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	StartCountDown();

}

void AFlag::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bActivated)return;
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (ActorOverlapped <= 0)StopCountDown();
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	HealthBarWidget->SetVisibility(false);
	HealthBarWidget->SetHealthPercent(0);
	bActivated = true;
}

void AFlag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if ((ActorOverlapped > 0) && bActivated)
	{
		Progress += DeltaTime * CaptureSpeed;
		HealthBarWidget->SetHealthPercent(Progress);
		if (Progress >=1.f && OverlappCharacter && HasAuthority())
		{
			MulticastAttach(OverlappCharacter);
		}
	}
}

void AFlag::StartCountDown()
{
	bStartCountdown = true;
	HealthBarWidget->SetVisibility(true);
	{ UE_LOG(LogTemp, Warning, TEXT("StartCountDown")); return; }
}

void AFlag::StopCountDown()
{
	bStartCountdown = false;
	HealthBarWidget->SetVisibility(false);
	Progress = 0;
}

void AFlag::Dropped()
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GetMesh()->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OverlappCharacter = nullptr;
	bActivated = true;
	Progress = 0;
	ATPSGameMode* GameMode = Cast<ATPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)GameMode->FlagDropped();
}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlag, bStartCountdown);
}

void AFlag::OnRep_StartCountdown()
{
	if(bStartCountdown == false)HealthBarWidget->SetVisibility(false);
	else if (bStartCountdown == true)HealthBarWidget->SetVisibility(true);
}

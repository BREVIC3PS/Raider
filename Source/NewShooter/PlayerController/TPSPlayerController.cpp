// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerController.h"
#include "NewShooter/HUD/CharacterHUD.h"
#include "NewShooter/HUD/CharacterOverlay.h"
#include "NewShooter/HUD/PreGameOverlay.h"
#include "NewShooter/HUD/CrosshairOverlay.h"
#include "NewShooter/NewShooterCharacter.h"
#include "NewShooter/PlayerState/TPSPlayerState.h"
#include "NewShooter/GameMode/TPSGameMode.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include "NewShooter/Dragon/Dragon.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/GameplayStatics.h>

//服务器调用
void ATPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	NewShooterCharacter = Cast<ANewShooterCharacter>(InPawn);
	DragonCharacter = Cast<ADragon>(InPawn);
	if (NewShooterCharacter)
	{
		OwningCharacter = NewShooterCharacter;
		DragonCharacter = nullptr;
	}
	else if (DragonCharacter) {
		NewShooterCharacter = nullptr;
	}

}

void ATPSPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay->HealthBar && HUD->CharacterOverlay->HealthText)
		{
			const float HealthPercent = Health / MaxHealth;
			HUD->CharacterOverlay->SetHealthBarPercent(HealthPercent);
			HUD->CharacterOverlay->SetHealthText(Health, MaxHealth);
		}
	}
}

void ATPSPlayerController::SetHUDScore(float Score)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay->ScoreAmount)
		{
			HUD->CharacterOverlay->SetScoreText(Score);
		}
	}
}

void ATPSPlayerController::SetHUDAmmo(int32 Ammo)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay->AmmoAmount)
		{
			HUD->CharacterOverlay->SetAmmoText(Ammo);
		}
	}
}

void ATPSPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay->CarriedAmmoAmount)
		{
			HUD->CharacterOverlay->SetCarriedAmmoText(CarriedAmmo);
		}
	}
}

void ATPSPlayerController::SetHUDReadyState(bool ReadyOrNot)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->PreGameOverlay)
	{
		if (HUD->PreGameOverlay->PlayerReadyText)
		{
			HUD->PreGameOverlay->SetReadyText(ReadyOrNot);
		}
	}
}

void ATPSPlayerController::SetHUDCrosshair()
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CrosshairOverlay)
	{
		HUD->DrawCorsshairWidget();
	}
}

void ATPSPlayerController::SetHUDDragonCrosshair(bool bShow)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CrosshairOverlay)
	{
		if (HUD->CrosshairOverlay)
		{
			HUD->CrosshairOverlay->ShowDragonOverlay(bShow);
		}
	}
}

void ATPSPlayerController::ShowHUDEnemyHit(bool bShow)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CrosshairOverlay)
	{
		APawn* ControlledPawn = GetPawn();
		NewShooterCharacter = Cast<ANewShooterCharacter>(ControlledPawn);
		DragonCharacter = Cast<ADragon>(ControlledPawn);
		if (NewShooterCharacter)
		{
			HUD->DrawCorsshairEnemyHit(bShow);
		}
		else if (DragonCharacter)
		{
			HUD->DrawCorsshairDragonEnemyHit(bShow);
		}
	}
}

void ATPSPlayerController::ShowHUDDragonEnemyHit(bool bShow)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CrosshairOverlay)
	{
		if (HUD->CrosshairOverlay)
		{
			HUD->CrosshairOverlay->ShowDragonHitOverlay(bShow);
		}
	}
}

void ATPSPlayerController::SetHUDDragonCrosshairPosition(FVector2D NewPos)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CrosshairOverlay)
	{
		if (HUD->CrosshairOverlay)
		{
			HUD->CrosshairOverlay->AdjustDragonCrosshair(NewPos);
		}
	}
}

void ATPSPlayerController::SetHUDTeamScore(TArray<int32>& TeamScores)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay)
		{
			HUD->CharacterOverlay->SetTeamScore(TeamScores);
		}
	}
}

void ATPSPlayerController::SetTeamRingPassedText(TArray<int32>& TeamPassRingCount)
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay)
	{
		if (HUD->CharacterOverlay)
		{
			HUD->CharacterOverlay->SetTeamRingPassedText(TeamPassRingCount);
		}
	}
}


void ATPSPlayerController::SetTeamId(const ETeamID& TeamID)
{
	if (PlayerState)
		Cast<ATPSPlayerState>(PlayerState)->SetTeamId(TeamID);
}

ETeamID ATPSPlayerController::GetTeamId()
{
	if (PlayerState)
		return Cast<ATPSPlayerState>(PlayerState)->GetTeamId();
	else return ETeamID::NoTeamId;

}

void ATPSPlayerController::ReadyButtonPressed()
{
	ServerPlayerReady(); //此处调用服务器RPC
}

void ATPSPlayerController::HitEnemy()
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD)
	{
		ShowHUDEnemyHit(true);
		GetWorldTimerManager().SetTimer(HUDChangeColorTimer, this, &ATPSPlayerController::HUDChangeColorFinished, 0.2f);
	}
}

void ATPSPlayerController::ClientHitEnemy_Implementation()
{
	HitEnemy();
}

void ATPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Add Input Mapping Context

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		// Ready
		EnhancedInputComponent->BindAction(ReadyAction, ETriggerEvent::Started, this, &ATPSPlayerController::ReadyButtonPressed);
	}

}

void ATPSPlayerController::SetHUDMatchStart()
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (HUD)
	{
		HUD->OnMatchStart();
	}
}

void ATPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ATPSPlayerController, NewShooterCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATPSPlayerController, DragonCharacter, COND_OwnerOnly)
}

//Server
void ATPSPlayerController::ServerPlayerReady_Implementation()
{
	bool ReadyOrNot = false;
	ATPSPlayerState* MyPlayerState = Cast<ATPSPlayerState>(PlayerState);
	if (MyPlayerState)
	{
		ReadyOrNot = MyPlayerState->GetPlayerReady();
		MyPlayerState->SetPlayerReady(!ReadyOrNot);
		ATPSGameMode* MyGameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
		if (MyGameMode)
		{
			MyGameMode->OnPlayerReady(this, !ReadyOrNot);//使用更新后的值
		}
	}

}

void ATPSPlayerController::ServerHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit, float Damage)
{
	ANewShooterCharacter* OwnerCharacter = Cast<ANewShooterCharacter>(GetPawn());
	if (OwnerCharacter)
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, this, OwnerCharacter, UDamageType::StaticClass());
	}
}

void ATPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<ACharacterHUD>(GetHUD());
	UE_LOG(LogTemp, Warning, TEXT("Player Controller BeginPlay"));
}

void ATPSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	if (GetPawn())
	{
		if (GetPawn()->GetController())
		{
			UE_LOG(LogTemp, Warning, TEXT("123"));
		}
	}
}

void ATPSPlayerController::OnRep_NewShooterCharacter()
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (NewShooterCharacter)
	{
		OwningCharacter = NewShooterCharacter;
		if (HUD && HUD->CrosshairOverlay) {
			HUD->CrosshairOverlay->ShowDragonOverlay(false);
			SetHUDHealth(NewShooterCharacter->GetHealth(), NewShooterCharacter->GetMaxHealth());
			if (NewShooterCharacter->GetEquippedWeapon())
			{
				HUD->DrawCorsshairWidget();
			}
		}
	}
}

void ATPSPlayerController::OnRep_DragonCharacter()
{
	HUD = HUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : HUD;
	if (DragonCharacter)
	{
		if (HUD && HUD->CrosshairOverlay)
		{
			HUD->CrosshairOverlay->ShowDragonOverlay(true);
			HUD->CrosshairOverlay->HideNormalOverlay();
		}
	}
}

void ATPSPlayerController::HUDChangeColorFinished()
{
	ShowHUDEnemyHit(false);
}

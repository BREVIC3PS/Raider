// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameMode.h"
#include "../PlayerController/TPSPlayerController.h"
#include "../NewShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "../GameState/TPSGameState.h"
#include "NewShooter/PlayerState/TPSPlayerState.h"
#include "../Pickup/Flag.h"
#include "../Pickup/FlyRing.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/GameSession.h"
#include "EngineUtils.h"
#include "../Weapon/Weapon.h"
#include "../Dragon/Dragon.h"
#include <NewShooter/Weapon/ProjectileWeapon_Base.h>
#include <AIController.h>

ATPSGameMode::ATPSGameMode()
{
	bDelayedStart = true;
}

void ATPSGameMode::StartMatch()
{
	
	Super::StartMatch();
}

FORCEINLINE int32 FromTeamIDtoIndex(ETeamID TeamId)
{
	return StaticCast<int32>(TeamId);
}



void ATPSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ATPSPlayerController* NewPlayerController = Cast<ATPSPlayerController>(NewPlayer);
	if (NewPlayerController)
	{
		ATPSGameState* MyGameState = GetGameState<ATPSGameState>();
		if (!MyGameState)return;
		TArray<int32> &TeamPlayers = MyGameState->TeamPlayers;
		int32 newId = 0;
		if (TeamPlayers.IsEmpty() || TeamPlayers.Num()<8)
		{
			TeamPlayers = { 0,0,0,0,0,0,0,0 };//初始化
		}
		// 只分配红队蓝队,哪个队人少分配哪个队

		newId = (TeamPlayers[FromTeamIDtoIndex(ETeamID::Red)] < TeamPlayers[FromTeamIDtoIndex(ETeamID::Blue)] ? FromTeamIDtoIndex(ETeamID::Red) : FromTeamIDtoIndex(ETeamID::Blue));

		NewPlayerController->SetTeamId(StaticCast<ETeamID>(newId));
		TeamPlayers[newId]++;
	}
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

//服务端调用
void ATPSGameMode::PlayerEliminated(ANewShooterCharacter* EliminatedCharacter, ATPSPlayerController* VictimController, ATPSPlayerController* AttackerController)
{
	ATPSPlayerState* AttackerPlayerState = AttackerController ? Cast<ATPSPlayerState>(AttackerController->PlayerState) : nullptr;
	ATPSPlayerState* VictimPlayerState = VictimController ? Cast<ATPSPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (EliminatedCharacter)
	{
		if (EliminatedCharacter->CarringFlag)
		{
			EliminatedCharacter->CarringFlag = nullptr;
			Flag->Dropped();
			FlagDropped();
		}
		EliminatedCharacter->Elim();
	}
}

//服务端调用
void ATPSGameMode::RequestRespawn(ANewShooterCharacter* EliminatedCharacter, AController* ElimmedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset(); 
		EliminatedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		RestartPlayer(ElimmedController);
	}
}

void ATPSGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	SpawnFlag();
	SpawnWeapon();
	SpawnDragon();
}

void ATPSGameMode::SpawnFlag()
{
	TArray<AActor*> SpawnLocations;

	FindSpawnLocation(SpawnLocations, "Flag");
	if (!SpawnLocations.IsEmpty())
	{
		int32 Selection = FMath::RandRange(0, SpawnLocations.Num() - 1);
		Flag = GetWorld()->SpawnActor<AFlag>(FlagClass, SpawnLocations[Selection]->GetActorLocation(), SpawnLocations[Selection]->GetActorRotation());
	}
}


void ATPSGameMode::SpawnWeapon()
{
	TArray<AActor*> SpawnLocations;

	FindSpawnLocation(SpawnLocations, "Weapon");
	if (!SpawnLocations.IsEmpty())
	{
		for (AActor* Actor : SpawnLocations)
		{
			AProjectileWeapon_Base* NewWeapon = GetWorld()->SpawnActor<AProjectileWeapon_Base>(SpawnedWeaponClass, Actor->GetActorLocation(), Actor->GetActorRotation());
			WeaponList.Add(NewWeapon);
		}
	}
}

void ATPSGameMode::SpawnDragon()
{
	TArray<AActor*> SpawnLocations;
	FindSpawnLocation(SpawnLocations, "Dragon");
	if (!SpawnLocations.IsEmpty())
	{
		for (AActor* Actor : SpawnLocations)
		{
			ADragon* NewDragon = GetWorld()->SpawnActor<ADragon>(SpawnedDragonClass, Actor->GetActorLocation(), Actor->GetActorRotation());
			DragonList.Add(NewDragon);
			AAIController* AIController = GetWorld()->SpawnActor<AAIController>(NewDragon->AIControllerClass, NewDragon->GetActorLocation(), NewDragon->GetActorRotation());
			AIController->Possess(NewDragon);
		}
	}
}

void ATPSGameMode::FindSpawnLocation(TArray<AActor*>& SpawnLocations, const FString& TagName /*= TEXT("")*/)
{
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		ATargetPoint* TargetPoint = Cast<ATargetPoint>(Actor);
		if (TargetPoint && TargetPoint->ActorHasTag(FName(TagName)))
		{
			SpawnLocations.Add(TargetPoint);
		}
	}
}

void ATPSGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	ATPSGameState* MyGameState = GetGameState<ATPSGameState>();
	if (!MyGameState)return;
	TArray<int32>& TeamRingCount = MyGameState->TeamPassRingCount;
	TeamRingCount = { 0,0,0,0,0,0,0,0 };
	TArray<int32>& TeamScore = MyGameState->TeamScores;
	TeamScore = { 0,0,0,0,0,0,0,0 };
	
}

void ATPSGameMode::OnPlayerReady(APlayerController* NewReadyPC, bool ReadyOrNot)
{
	ATPSPlayerController* NewPlayerController = Cast<ATPSPlayerController>(NewReadyPC);
	if (NewPlayerController)
	{
		ReadyOrNot ? NumPlayerReady++ : NumPlayerReady--;
		UE_LOG(LogTemp, Warning, TEXT("Player ready: %d"), NumPlayerReady);
		if (NumPlayerReady == NumPlayers && NumPlayers>0) {
			StartMatch();
		}
	}
}

void ATPSGameMode::PlayerWalkThroughRing(ATPSPlayerController* PC)
{
	if (PC)
	{
		ATPSGameState* MyGameState = GetGameState<ATPSGameState>();
		if (!MyGameState)return;
		ETeamID PlayerTeamID = PC->GetTeamId();
		TArray<int32>& TeamRingCount = MyGameState->TeamPassRingCount;
		TeamRingCount[FromTeamIDtoIndex(PlayerTeamID)]++;
		UE_LOG(LogTemp, Warning, TEXT("TeamRingCount for Team%d: %d"), PlayerTeamID, TeamRingCount[FromTeamIDtoIndex(PlayerTeamID)]);
		if (TeamRingCount[FromTeamIDtoIndex(PlayerTeamID)] == 4)
		{
			TArray<int32>& TeamScore = MyGameState->TeamScores;
			TeamScore[FromTeamIDtoIndex(PlayerTeamID)]++;
			UE_LOG(LogTemp, Warning, TEXT("TeamRingScore for Team%d: %d"), PlayerTeamID, TeamScore[FromTeamIDtoIndex(PlayerTeamID)]);
			GetWorldTimerManager().SetTimer(ResetRingTimer, this, &ATPSGameMode::ResetRingTimerFinished, 3, false);
			if (TeamScore[FromTeamIDtoIndex(PlayerTeamID)] > 2)
			{
				GetWorldTimerManager().SetTimer(RestartTimer, this, &ATPSGameMode::RestartTimerFinished, 5, false);
			}
		}
	}
}

void ATPSGameMode::SetRingCountToZero()
{
	ATPSGameState* MyGameState = GetGameState<ATPSGameState>();
	if (!MyGameState)return;
	TArray<int32>& TeamRingCount = MyGameState->TeamPassRingCount;
	for (int32 i = 0; i < 8; i++) {

		TeamRingCount[i] = 0;
		UE_LOG(LogTemp, Warning, TEXT("TeamRingCount for Team%d: %d"), i, TeamRingCount[i]);
	}
}
void ATPSGameMode::ResetAllRings()
{
	SetRingCountToZero();
	TArray<AActor*> AllFlagActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFlyRing::StaticClass(), AllFlagActors);
	for (AActor* Actor : AllFlagActors)
	{
		AFlyRing* Ring = Cast<AFlyRing>(Actor);
		if (Ring)
		{
			Ring->ReActivate();
		}
	}
}

void ATPSGameMode::RestartTimerFinished()
{
	RestartGame();
}

void ATPSGameMode::ResetRingTimerFinished()
{
	// 获取所有的Actor
	RestartLevel();
	
	
}
void ATPSGameMode::RestartGame()
{
	//if (GameSession->CanRestartGame())
	//{
	//	if (GetMatchState() == MatchState::LeavingMap)
	//	{
	//		return;
	//	}

	//	GetWorld()->ServerTravel("?Restart", false);
	//}
	Super::RestartGame();
}
void ATPSGameMode::FlagDropped()
{
	UWorld* World = GetWorld();
	for (TActorIterator<ANewShooterCharacter> It(World); It; ++It)
	{
		ANewShooterCharacter* PlayerCharacter = *It;
		if (PlayerCharacter)
		{
			PlayerCharacter->CarringFlag = nullptr;
		}
	}
	Flag->Destroy();
	SpawnFlag();

	ResetAllRings();

}

void ATPSGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}
	ATPSPlayerState* PlayerState = NewPlayer->GetPlayerState<ATPSPlayerState>();
	AActor* StartSpot = nullptr;
	if (PlayerState->GetTeamId() == ETeamID::Red)
	{
		StartSpot = FindPlayerStart(NewPlayer, TEXT("Red"));
	}
	else if (PlayerState->GetTeamId() == ETeamID::Blue)
	{
		StartSpot = FindPlayerStart(NewPlayer, TEXT("Blue"));
	}
	if (StartSpot == nullptr)
	{
		StartSpot = FindPlayerStart(NewPlayer);
	}
	// If a start spot wasn't found,
	if (StartSpot == nullptr)
	{
		// Check for a previously assigned spot
		if (NewPlayer->StartSpot != nullptr)
		{
			StartSpot = NewPlayer->StartSpot.Get();
			UE_LOG(LogGameMode, Warning, TEXT("RestartPlayer: Player start not found, using last start spot"));
		}
	}

	RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}

void ATPSGameMode::RestartLevel()
{

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANewShooterCharacter::StaticClass(), AllActors);
	for (AActor* Actor : AllActors)
	{
		ANewShooterCharacter* PlayerCharacter = Cast<ANewShooterCharacter>(Actor);
		if (PlayerCharacter)
		{
			PlayerCharacter->ServerDismount();
			RequestRespawn(PlayerCharacter, PlayerCharacter->Controller);
		}
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeapon::StaticClass(), AllActors);
	for (AActor* Actor : AllActors)
	{
		AWeapon* Weapon = Cast<AWeapon>(Actor);
		if (Weapon)
		{
			Weapon->Dropped();
			Weapon->Destroy();
		}
	}
	
	for (ADragon* Dragon : DragonList)
	{
		if (Dragon)
			Dragon->Destroy();
	}

	WeaponList.Empty();
	DragonList.Empty();
	
	FlagDropped(); // Call ResetAllRings();
	SpawnWeapon();
	SpawnDragon();

}



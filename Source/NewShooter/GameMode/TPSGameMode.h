// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GenericTeamAgentInterface.h"
#include "TPSGameMode.generated.h"


/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ATPSGameMode : public AGameMode
{
	GENERATED_BODY()
	
private:
	ATPSGameMode();

	UPROPERTY(VisibleAnywhere)
	int32 NumPlayerReady = 0;

	virtual void StartMatch() override;

	class AFlag* Flag;

	FTimerHandle RestartTimer;
	FTimerHandle ResetRingTimer;

	void RestartTimerFinished();
	void ResetRingTimerFinished();

	void ResetAllRings();

	virtual void RestartGame() override;

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InitialSpawnedActor")
	TSubclassOf<AFlag> FlagClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InitialSpawnedActor")
	TSubclassOf<class AWeapon> SpawnedWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InitialSpawnedActor")
	TSubclassOf<class ADragon> SpawnedDragonClass;

	TArray<AWeapon*> WeaponList;

	TArray<ADragon*> DragonList;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	virtual void PlayerEliminated(class ANewShooterCharacter* EliminatedCharacter, class ATPSPlayerController* VictimController, ATPSPlayerController* AttackerController);
	
	virtual void RequestRespawn(ANewShooterCharacter* EliminatedCharacter, AController* ElimmedController);
	
	virtual void HandleMatchHasStarted() override;

	void SpawnFlag();

	void FindSpawnLocation(TArray<AActor*>& SpawnLocations, const FString& TagName = TEXT(""));

	void SpawnWeapon();

	void SpawnDragon();

	virtual void HandleMatchIsWaitingToStart() override;

	void OnPlayerReady(APlayerController* NewReadyPC, bool ReadyOrNot);

	void PlayerWalkThroughRing(ATPSPlayerController* PC);

	void SetRingCountToZero();

	void FlagDropped();

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void RestartLevel();
};

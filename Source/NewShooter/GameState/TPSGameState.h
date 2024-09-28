// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPSGameState.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ATPSGameState : public AGameState
{
	GENERATED_BODY()
	
	
public:
	virtual void OnRep_MatchState() override;

	UFUNCTION()
	virtual void OnRep_TeamScores();
	UFUNCTION()
	virtual void OnRep_TeamPassRingCount();

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<int32> TeamPlayers;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_TeamScores)
	TArray<int32> TeamScores;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_TeamPassRingCount)
	TArray<int32> TeamPassRingCount;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

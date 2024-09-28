// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../NewShooter.h"
#include "TPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ATPSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Score() override;
	virtual void AddToScore(float ScoreAmount);
	void SetHUDScore(float NewScoreAmount);

	/** Assigns Team Agent to given TeamID */
	virtual void SetTeamId(const ETeamID& TeamID) { TeamId = TeamID; }

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual ETeamID GetTeamId() const { return TeamId; }

	UFUNCTION()
	void OnRep_PlayerReady();

	FORCEINLINE void SetPlayerReady(bool ReadyOrNot) { PlayerReady = ReadyOrNot; }
	FORCEINLINE bool GetPlayerReady() const { return PlayerReady; }
private:
	UPROPERTY()
	class ANewShooterCharacter* Character = nullptr;
	UPROPERTY()
	class ATPSPlayerController* Controller = nullptr;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	ETeamID TeamId;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_PlayerReady)
	bool PlayerReady;
	
protected:
	virtual void BeginPlay() override;
};

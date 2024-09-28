// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerState.h"
#include "../NewShooterCharacter.h"
#include "../PlayerController/TPSPlayerController.h"
#include <Net/UnrealNetwork.h>

void ATPSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSPlayerState,TeamId);
	DOREPLIFETIME(ATPSPlayerState, PlayerReady);
}

// Rep函数客户端调用
void ATPSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	SetHUDScore(GetScore());
}

//服务端和客户端出生时调用
void ATPSPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	SetHUDScore(GetScore());
}

void ATPSPlayerState::SetHUDScore(float NewScoreAmount)
{
	Controller = Controller == nullptr ? Cast<ATPSPlayerController>(GetPlayerController()) : Controller;
	if (Controller)
	{
		Controller->SetHUDScore(NewScoreAmount);
	}
}

// Rep函数客户端调用
void ATPSPlayerState::OnRep_PlayerReady()
{
	Controller = Controller == nullptr ? Cast<ATPSPlayerController>(GetPlayerController()) : Controller;
	if (Controller)
	{
		if (Controller->PlayerState == this)//确保是当前的PlayerState
		{
			Controller->SetHUDReadyState(PlayerReady);
		}
	}
}

void ATPSPlayerState::BeginPlay()
{
	Super::BeginPlay();
}


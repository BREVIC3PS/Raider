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

// Rep�����ͻ��˵���
void ATPSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	SetHUDScore(GetScore());
}

//����˺Ϳͻ��˳���ʱ����
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

// Rep�����ͻ��˵���
void ATPSPlayerState::OnRep_PlayerReady()
{
	Controller = Controller == nullptr ? Cast<ATPSPlayerController>(GetPlayerController()) : Controller;
	if (Controller)
	{
		if (Controller->PlayerState == this)//ȷ���ǵ�ǰ��PlayerState
		{
			Controller->SetHUDReadyState(PlayerReady);
		}
	}
}

void ATPSPlayerState::BeginPlay()
{
	Super::BeginPlay();
}


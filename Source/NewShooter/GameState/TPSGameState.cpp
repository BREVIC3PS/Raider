// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameState.h"
#include <Net/UnrealNetwork.h>
#include <GameFramework/GameMode.h>
#include "../PlayerController/TPSPlayerController.h"

//客户端调用
void ATPSGameState::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		if (GetWorld())
		{
			ATPSPlayerController* Controller = GetWorld()->GetFirstPlayerController<ATPSPlayerController>();
			if (Controller)
			{
				Controller->SetHUDMatchStart();
			}
		}
	}
	Super::OnRep_MatchState();
}

void ATPSGameState::OnRep_TeamScores()
{
	if (GetWorld()&&!TeamScores.IsEmpty())
	{
		ATPSPlayerController* Controller = GetWorld()->GetFirstPlayerController<ATPSPlayerController>();
		if (Controller)
		{
			Controller->SetHUDTeamScore(TeamScores);
		}
	}
}

void ATPSGameState::OnRep_TeamPassRingCount()
{
	if (GetWorld() && !TeamPassRingCount.IsEmpty())
	{
		ATPSPlayerController* Controller = GetWorld()->GetFirstPlayerController<ATPSPlayerController>();
		if (Controller)
		{
			Controller->SetTeamRingPassedText(TeamPassRingCount);
		}
	}
}

void ATPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPSGameState, TeamPlayers);
	DOREPLIFETIME(ATPSGameState, TeamScores);
	DOREPLIFETIME(ATPSGameState, TeamPassRingCount);
}

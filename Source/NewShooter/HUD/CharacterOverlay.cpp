// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "GameFramework/PlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCharacterOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}

void UCharacterOverlay::SetHealthText(float Health, float MaxHealth)
{
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth))));
	}
}

void UCharacterOverlay::SetScoreText(float NewScore)
{
	if (ScoreAmount)
	{
		ScoreAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(NewScore))));
	}
}

void UCharacterOverlay::SetAmmoText(int32 NewAmmo)
{
	if (AmmoAmount)
	{
		AmmoAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewAmmo)));
	}
}

void UCharacterOverlay::SetCarriedAmmoText(int32 NewCarriedAmmo)
{
	if (CarriedAmmoAmount)
	{
		CarriedAmmoAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewCarriedAmmo)));
	}
}

void UCharacterOverlay::SetTeamRingPassedText(TArray<int32>& TeamPassRingCount)
{
	if (Team1RingPassed)
	{
		Team1RingPassed->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamPassRingCount[1])));
	}
	if (Team2RingPassed)
	{
		Team2RingPassed->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamPassRingCount[2])));
	}
}

void UCharacterOverlay::SetTeamScore(TArray<int32>& TeamScores)
{
	if (Team1Score)
	{
		if (TeamScores[1] == 3)
		{
			Team1Score->SetText(FText::FromString(FString::Printf(TEXT("Win!"))));
		}
		else Team1Score->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamScores[1])));
	}
	if (Team2Score)
	{
		if (TeamScores[2] == 3)
		{
			Team2Score->SetText(FText::FromString(FString::Printf(TEXT("Win!"))));
		}
		else Team2Score->SetText(FText::FromString(FString::Printf(TEXT("%d"), TeamScores[2])));
	}
}

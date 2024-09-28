// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta= (BindWidget))
	class UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team1RingPassed;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team2RingPassed;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team1Score;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Team2Score;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	void SetHealthBarPercent(float Percent);
	void SetHealthText(float Health, float MaxHealth);
	void SetScoreText(float NewScore);
	void SetAmmoText(int32 NewAmmo);
	void SetCarriedAmmoText(int32 NewAmmo);
	void SetTeamRingPassedText(TArray<int32>& TeamPassRingCount);
	void SetTeamScore(TArray<int32> &TeamScores);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairOverlay.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API UCrosshairOverlay : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	class UImage* NormalCrosshair;

	UPROPERTY(meta = (BindWidget))
	UImage* EnemyHitCrosshair;

	UPROPERTY(meta = (BindWidget))
	UImage* DragonCrosshair;

	UPROPERTY(meta = (BindWidget))
	UImage* DragonHitCrosshair;
	
public:
	void ShowNormalOverlay();
	void HideNormalOverlay();
	void ShowEnemyHitOverlay(bool bShow);
	void ShowDragonHitOverlay(bool bShow);
	void ShowDragonOverlay(bool bShow);
	void AdjustDragonCrosshair(FVector2D NewPos);

	FVector2D GetCanvasSize();
	
};

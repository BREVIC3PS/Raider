// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterOverlay.h"
#include "CharacterHUD.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()

public: 
	virtual void DrawHUD() override;

	class UTexture2D* CrosshairsCenter;
	FLinearColor CrosshairsColor;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UCharacterOverlay> OverlayClass;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPreGameOverlay> PreGameOverlayClass;

	UPROPERTY()
	UPreGameOverlay* PreGameOverlay;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UCrosshairOverlay> CrosshairOverlayClass;

	UPROPERTY()
	UCrosshairOverlay* CrosshairOverlay;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FLinearColor CrosshairColor);

	void DrawCorsshairWidget();
	void DrawCorsshairEnemyHit(bool bDraw);

	void DrawCorsshairDragonEnemyHit(bool bDraw);

	void OnMatchStart();
	
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
	float CrosshairsCenterScale = 1.f;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EBlendMode> CrosshairsCenterBlendMode = BLEND_Translucent;

};

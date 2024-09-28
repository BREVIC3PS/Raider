// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include <NewShooter/NewShooter.h>
#include "PreGameOverlay.generated.h"
/**
 * 
 */
UCLASS()
class NEWSHOOTER_API UPreGameOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerReadyText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TeanText;

	void SetReadyText(bool State);
	void SetTeamText(ETeamID TeamId);
	
};

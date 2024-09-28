// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarUserWidget.h"
#include "HealthBarWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API UHealthBarWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
public:
	UHealthBarWidgetComponent();
	void SetHealthPercent(float Percent);
private:

	UPROPERTY()
	UHealthBarUserWidget* HealthBarUserWidget;
	
};

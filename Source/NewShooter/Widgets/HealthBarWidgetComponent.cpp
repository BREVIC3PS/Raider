// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBarWidgetComponent.h"
#include "Components/ProgressBar.h"

void UHealthBarWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarUserWidget == nullptr)
	{
		HealthBarUserWidget = Cast<UHealthBarUserWidget>(GetUserWidgetObject());
		if (!HealthBarUserWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Begin play: WealthBarWidget is empty"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Begin play: WealthBarWidget is not empty"));
		}
	}
}

UHealthBarWidgetComponent::UHealthBarWidgetComponent()
{
}

void UHealthBarWidgetComponent::SetHealthPercent(float Percent)
{
	if (HealthBarUserWidget && HealthBarUserWidget->HealthBar)
	{
		HealthBarUserWidget->HealthBar->SetPercent(Percent);
	}
}

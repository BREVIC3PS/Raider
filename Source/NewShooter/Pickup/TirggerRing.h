// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "TirggerRing.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ATirggerRing : public ATriggerBox
{
	GENERATED_BODY()
public:
	ATirggerRing();

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);
	
	
};

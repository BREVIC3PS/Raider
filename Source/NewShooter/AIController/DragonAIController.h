// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DragonAIController.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ADragonAIController : public AAIController
{
	GENERATED_BODY()
	
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	virtual void OnUnPossess() override;
	
};

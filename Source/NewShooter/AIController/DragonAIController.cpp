// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonAIController.h"
#include "../Dragon/Dragon.h"

void ADragonAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	/*ADragon* OwningDragon = Cast<ADragon>(GetPawn());
	if (OwningDragon)
	{
		UnPossess();
		OwningDragon->Destroy();
		Destroy();
	}*/
}

void ADragonAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonAnimInstance.h"
#include "../Dragon/Dragon.h"

void UDragonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Dragon = Cast<ADragon>(TryGetPawnOwner());
}

void UDragonAnimInstance::NativeUpdateAnimation(float DeltaTime)
{

	if (Dragon == nullptr)
	{
		Dragon = Cast<ADragon>(TryGetPawnOwner());
	}
	if (Dragon == nullptr) return;

	LeanX = Dragon->GetLeanX();
	LeanY = Dragon->GetLeanY();
	Speed = Dragon->GetVelocity().Length();
	bFastFlight = Dragon->GetFastFlight();
}

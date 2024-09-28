// Copyright Epic Games, Inc. All Rights Reserved.

#include "NewShooterGameMode.h"
#include "NewShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANewShooterGameMode::ANewShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

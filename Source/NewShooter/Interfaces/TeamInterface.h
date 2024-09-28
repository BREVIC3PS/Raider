// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../NewShooter.h"

#include "TeamInterface.generated.h"



// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTeamInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NEWSHOOTER_API ITeamInterface
{
	GENERATED_BODY()
private:

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Assigns Team Agent to given TeamID */
	virtual void SetTeamId(const ETeamID& TeamID) = 0;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual ETeamID GetTeamId() = 0;

	
};

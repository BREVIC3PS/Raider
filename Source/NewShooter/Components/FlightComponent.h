// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlightComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEWSHOOTER_API UFlightComponent : public UActorComponent
{
	GENERATED_BODY()
private:

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	//UInputAction* TurnAction;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	//UInputAction* JumpAction;

	UPROPERTY(EditAnywhere)
	bool bFlyModeMouseControl = true;

	UPROPERTY(EditAnywhere)
	bool bFlyModeKeyboardControl = false;

	UPROPERTY(VisibleAnywhere)
	float Left_RightAxis = 0;

	UPROPERTY(VisibleAnywhere)
	float Forward_BackwardAxis = 0;

	UPROPERTY(VisibleAnywhere)
	float Up_DownAxis = 0;

	UPROPERTY(VisibleAnywhere)
	float FlightPitch = 0;

	UPROPERTY(VisibleAnywhere)
	float FlightTurn = 0;

	UPROPERTY(VisibleAnywhere)
	float FlightYaw = 0;

	UPROPERTY(EditAnywhere)
	float FlightTurnRate = 20;

	float WorldDeltaSecond = 0;

	UPROPERTY(VisibleAnywhere)
	FRotator FlightRotation;

	UPROPERTY(EditAnywhere)
	float MaxFlySpeed = 1000;

	UPROPERTY(EditAnywhere)
	bool bLeaning = true;;

	UPROPERTY(EditAnywhere)
	float DefaultFOV = 90;

	UPROPERTY(EditAnywhere)
	float FastFlightFOV = 120;

	UPROPERTY(VisibleAnywhere)
	float DesiredFOV = DefaultFOV;

	UPROPERTY(EditAnywhere)
	float FOVInterpSpeed = 6;

	UPROPERTY(EditAnywhere)
	float FlghtRotationInterpSpeed = 2;

	UPROPERTY(EditAnywhere)
	float FlghtRotationInterpConstantSpeed = 800;

	UPROPERTY(EditAnywhere)
	FVector3d DefaultSocketOffset = { 0,200,0 };

	UPROPERTY(EditAnywhere)
	FVector3d FastFlightSocketOffset = { 150,0,90 };

	UPROPERTY(VisibleAnywhere)
	FVector3d DesiredSocketOffset = DefaultSocketOffset;


	UPROPERTY(VisibleAnywhere)
	FRotator PreviousRotation = { 0,0,0 };

	FRotator LastVelocityRotation;
	FRotator CalculateFlightRotation();

	void UpdateActorRotationSmoothly(FRotator Target, float ConstantSpeed, float SmoothSpeed);

public:	
	// Sets default values for this component's properties
	UFlightComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY()
	class ADragon* Character;

	UPROPERTY()
	class ATPSPlayerController* TPSController;

	class AController* Controller;
		
	UPROPERTY(EditAnywhere)
	float FallingThreshold = 100.f;


	UPROPERTY(VisibleAnywhere)
	float LeanY = 0;

	UPROPERTY(VisibleAnywhere)
	float LeanX = 0;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool FastFlight = false;

	UFUNCTION(Server, Unreliable)
	void ServerSetRotation();

	UFUNCTION(Server, reliable)
	void ServerSetFastFly(bool bFastfly);
	
	void SetFastFly(bool bFastfly);
	void Acclerate(float value);
	void FlyControl(FVector MovementVector);
	void MoveStop();
	void RotateYaw(float value);
	void CalculateFlightLean();
};

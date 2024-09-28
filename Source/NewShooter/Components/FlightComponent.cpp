// Fill out your copyright notice in the Description page of Project Settings.


#include "FlightComponent.h"
#include "../NewShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "../PlayerController/TPSPlayerController.h"
#include "../Dragon/Dragon.h"
#include <Net/UnrealNetwork.h>


FRotator UFlightComponent::CalculateFlightRotation()
{
	if (FastFlight)
	{
		return Character->GetControlRotation();
	}
	FVector FlightRotationVector;
	if (Character->GetVelocity().Length() > 1.f)
	{
		FVector LastVelocityRollYawVector = UKismetMathLibrary::MakeVector(LastVelocityRotation.Roll, 0.f, LastVelocityRotation.Yaw);
		if (LastVelocityRotation.Yaw == 0)
		{
			LastVelocityRollYawVector.Z = Character->GetActorRotation().Yaw;
		}
		FlightRotationVector = LastVelocityRollYawVector;
	}
	else {
		FVector ActorRotationRollYawVector = UKismetMathLibrary::MakeVector(Character->GetActorRotation().Roll, 0.f, Character->GetActorRotation().Yaw);
		FlightRotationVector = ActorRotationRollYawVector;
	}
	return UKismetMathLibrary::MakeRotator(FlightRotationVector.X, FlightRotationVector.Y, FlightRotationVector.Z);

}

void UFlightComponent::UpdateActorRotationSmoothly(FRotator Target, float ConstantSpeed, float SmoothSpeed)
{

	FlightRotation = UKismetMathLibrary::RInterpTo_Constant(FlightRotation, Target, WorldDeltaSecond, ConstantSpeed);
	FRotator NewActorRotation = UKismetMathLibrary::RInterpTo(Character->GetActorRotation(), FlightRotation, WorldDeltaSecond, SmoothSpeed);
	Character->SetActorRotation(NewActorRotation);

}

// Sets default values for this component's properties
UFlightComponent::UFlightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UFlightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UFlightComponent, FastFlight);
	DOREPLIFETIME_CONDITION(UFlightComponent, FastFlight, COND_SimulatedOnly);
	//DOREPLIFETIME(UFlightComponent, Controller);
}

// Called when the game starts
void UFlightComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Character is nullptr"));
		return;
	}
	TPSController = TPSController == nullptr ? Cast<ATPSPlayerController>(Character->Controller) : TPSController;
	Character = Character == nullptr ? Cast<ADragon>(Character) : Character;
	SetFastFly(false);
}


// Called every frame
void UFlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	WorldDeltaSecond = DeltaTime;

	if (!Character)return;
	if (bFlyModeMouseControl)
	{
		if (bLeaning)
		{
			CalculateFlightLean();
		}

		LastVelocityRotation = UKismetMathLibrary::Conv_VectorToRotator(Character->GetVelocity());

		if (!Character->IsLocallyControlled() && !Character->HasAuthority())
		{
			return;
		}
		if (Character->GetFollowCamera())
		{
			float UpdatedFOV = UKismetMathLibrary::FInterpTo(Character->GetFollowCamera()->FieldOfView, DesiredFOV, DeltaTime, FOVInterpSpeed);
			Character->GetFollowCamera()->SetFieldOfView(UpdatedFOV);

			FVector UpdatedOffset = UKismetMathLibrary::VInterpTo(Character->GetCameraBoom()->SocketOffset, DesiredSocketOffset, DeltaTime, 3.0f);
			Character->GetCameraBoom()->SocketOffset = UpdatedOffset;
		}

		if (Character->GetCharacterMovement()->IsFlying())
		{
			
			UpdateActorRotationSmoothly(CalculateFlightRotation(), FlghtRotationInterpConstantSpeed, FlghtRotationInterpSpeed);
			/*if (FastFlight)
			{
				if (Character->HasAuthority())
				{
					UE_LOG(LogTemp, Warning, TEXT("Server: %s"), *Character->GetActorRotation().ToString());
				}
				else if (Character->IsLocallyControlled())
				{
					UE_LOG(LogTemp, Warning, TEXT("Local Client: %s"), *Character->GetActorRotation().ToString());
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Sim Client: %s"), *Character->GetActorRotation().ToString());
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Last Velocity Rotation: %s"), *LastVelocityRotation.ToString());
			}*/
		}
	}
}


void UFlightComponent::ServerSetFastFly_Implementation(bool bFastfly)
{
	SetFastFly(bFastfly);
}

void UFlightComponent::SetFastFly(bool bFastfly)
{
	if (!Character)return;
	if (bFastfly)
	{
		FastFlight = true;
		DesiredFOV = FastFlightFOV;
		DesiredSocketOffset = FastFlightSocketOffset;
		FOVInterpSpeed = 10;
		Character->GetCharacterMovement()->BrakingDecelerationFlying = 15000;
		Character->GetCharacterMovement()->MaxAcceleration = 15000;
		Character->GetCharacterMovement()->MaxFlySpeed = 4000;
	}
	else
	{
		FastFlight = false;
		DesiredFOV = DefaultFOV;
		DesiredSocketOffset = DefaultSocketOffset;
		FOVInterpSpeed = 0.6;
		Character->GetCharacterMovement()->BrakingDecelerationFlying = 6000;
		Character->GetCharacterMovement()->MaxAcceleration = 6000;
		Character->GetCharacterMovement()->MaxFlySpeed = 2000;
	}
	if (!Character->HasAuthority() && Character->IsLocallyControlled())
	{
		ServerSetFastFly(bFastfly);
	}
}

void UFlightComponent::Acclerate(float value)
{
	if (!Character)return;
	if (Character->GetCharacterMovement()->IsFlying())
	{
		// input is a float

		if (TPSController != nullptr)
		{
			// find out which way is forward
			const FRotator Rotation = Character->GetActorRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = Character->GetActorForwardVector();;

			// add movement 
			Character->AddMovementInput(ForwardDirection, value);
		}
	}
}


void UFlightComponent::RotateYaw(float value)
{
	if (!Character)return;
	if (Character->GetCharacterMovement()->IsFlying())
	{
		// input is a float

		if (TPSController != nullptr)
		{
			//设置Yaw的旋转
			FlightYaw = UKismetMathLibrary::FInterpTo(FlightYaw, value * FlightTurnRate, WorldDeltaSecond, 2.f);
			float DeltaRotationYaw = (FlightYaw * WorldDeltaSecond * FlightTurnRate);
			Character->AddActorLocalRotation(FRotator(0, DeltaRotationYaw, 0));
		}
	}
}

void UFlightComponent::CalculateFlightLean()
{
	if (FastFlight)
	{
		float TargetY = UKismetMathLibrary::MapRangeClamped(
			UKismetMathLibrary::NormalizedDeltaRotator(LastVelocityRotation,
				PreviousRotation).Pitch / WorldDeltaSecond, -180, 180, -1, 1);

		float TargetX = UKismetMathLibrary::MapRangeClamped(
			UKismetMathLibrary::NormalizedDeltaRotator(LastVelocityRotation,
				PreviousRotation).Yaw / WorldDeltaSecond, -180, 180, -1, 1);

		LeanY = UKismetMathLibrary::FInterpTo(LeanY, TargetY, WorldDeltaSecond, 15);
		LeanX = UKismetMathLibrary::FInterpTo(LeanX, TargetX, WorldDeltaSecond, 5);
		PreviousRotation = LastVelocityRotation;
	}
}

void UFlightComponent::FlyControl(FVector MovementVector)
{
	Controller = Character->Controller;
	if(!Controller) { UE_LOG(LogTemp, Warning, TEXT("FlightComponent->Controller is empty")); return; }
	Left_RightAxis = MovementVector.X;
	Forward_BackwardAxis = MovementVector.Y;
	Up_DownAxis = MovementVector.Z;

	if (bFlyModeKeyboardControl)
	{
		//设置Pitch的旋转
		FlightPitch = UKismetMathLibrary::FInterpTo(FlightPitch, -Forward_BackwardAxis * FlightTurnRate, WorldDeltaSecond, 2.f);
		float DeltaRotationPitch = (FlightPitch * WorldDeltaSecond * FlightTurnRate);
		Character->AddActorLocalRotation(FRotator(DeltaRotationPitch, 0, 0));

		//设置Roll的旋转
		FlightTurn = UKismetMathLibrary::FInterpTo(FlightTurn, Left_RightAxis * FlightTurnRate, WorldDeltaSecond, 2.f);
		float DeltaRotationRoll = (FlightTurn * WorldDeltaSecond * 20.f);
		Character->AddActorLocalRotation(FRotator(0, 0, DeltaRotationRoll));
	}
	else if (bFlyModeMouseControl)
	{
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		if (!FastFlight)
		{

			//FVector Left_RightDirection = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotator(0, 0, Character->GetControlRotation().Yaw));
			FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			Character->AddMovementInput(RightDirection, Left_RightAxis);

			FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			Character->AddMovementInput(ForwardDirection, Forward_BackwardAxis);

			FVector VerticalDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);
			Character->AddMovementInput(VerticalDirection, Up_DownAxis);
		}
		else
		{
			FVector FlyDirection = UKismetMathLibrary::Conv_RotatorToVector(Rotation);
			Character->AddMovementInput(FlyDirection, 1);
		}
	}



}

void UFlightComponent::MoveStop()
{
	FlightPitch = 0;
	FlightTurn = 0;

}


void UFlightComponent::ServerSetRotation_Implementation()
{
}
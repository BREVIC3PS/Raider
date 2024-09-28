// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSAnimInstance.h"
#include "NewShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"

void UTPSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	NewShooterCharacter = Cast<ANewShooterCharacter>(TryGetPawnOwner());
}

void UTPSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (NewShooterCharacter == nullptr)
	{
		NewShooterCharacter = Cast<ANewShooterCharacter>(TryGetPawnOwner());
	}
	if (NewShooterCharacter == nullptr) return;

	FVector Velocity = NewShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = NewShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = NewShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = NewShooterCharacter->IsWeaponEquipped();
	EquippedWeapon = NewShooterCharacter->GetEquippedWeapon();
	bIsCrouched = NewShooterCharacter->bIsCrouched;
	bAiming = NewShooterCharacter->IsAiming();
	bRotateRootBone = NewShooterCharacter->ShouldRotateRootBone();
	bElimmed = NewShooterCharacter->IsElimmed();
	//TurningInPlace = NewShooterCharacter->GetTurningInPlace();

	// Offset Yaw for Strafing
	FRotator AimRotation = NewShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(NewShooterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	//Leaning is not used
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = NewShooterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = NewShooterCharacter->GetAO_Yaw();
	AO_Pitch = NewShooterCharacter->GetAO_Pitch();

	CharacterState = NewShooterCharacter->GetCharacterState();

	//if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && NewShooterCharacter->GetMesh())
	//{
	//	LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
	//	FVector OutPosition;
	//	FRotator OutRotation;
	//	NewShooterCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
	//	LeftHandTransform.SetLocation(OutPosition);
	//	LeftHandTransform.SetRotation(FQuat(OutRotation));

	//	if (NewShooterCharacter->IsLocallyControlled())
	//	{
	//		bLocallyControlled = true;
	//		//FTransform RightHandTransform = NewShooterCharacter->GetMesh()->GetSocketTransform(FName("RightHand"), ERelativeTransformSpace::RTS_World);
	//		/*FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - NewShooterCharacter->GetHitTarget()));
	//		RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);*/
	//	}
	//}
}
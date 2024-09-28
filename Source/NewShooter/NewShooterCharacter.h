// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NewShooter.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "NewShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ANewShooterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* StopFireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AccelerationAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateYawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CallDragonAction;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(EditAnywhere, Replicated)
	class ADragon* OverlappingDragon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UFlightComponent* FlightComponent;*/

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	UPROPERTY(EditAnywhere)
	float SprintSpeed;

	/*ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);*/

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> BeginningWeapon;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	bool bRotateRootBone;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 1.f;

	void ElimTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats", ReplicatedUsing = OnRep_Health)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UHealthBarWidgetComponent* HealthBarWidget;

	

	virtual void Jump() override;

	void SpawnFlyingPlatform();

	UPROPERTY(ReplicatedUsing = OnRep_CharacterState, VisibleAnywhere)
	ECharacterState CharacterState;

	UFUNCTION()
	void OnRep_CharacterState();

	float LastCameraLocation;
	float LastCameraRotation;

	UPROPERTY(Replicated)
	float ReplicatedControlRotationYaw;


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveButtonReleased();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void AccButtonPressed(const FInputActionValue& Value);

	void SetCharacterSprinting(bool Acc);

	void RotateYawButtonPressed(const FInputActionValue& Value);

	void EquipButtonPressed();
	void CrouchButtonPressed();

	/** Called for firing input */
	void FireButtonPressed();
	void FireButtonReleased();

	void ReloadButtonPressed();

	void CallDragonPressed();
	ADragon* FindNearestDragon(const FVector& TargetLocation);
	UFUNCTION(Server, Reliable)
	void ServerSummonNearestDragon(ADragon* NearestDragon);

	void Aim();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	void PlayHitReactMontage();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

	void HideHealthBarIfIsEnemyOrSelf();

	void UpdateHUDHealth();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	UPROPERTY()
	class ATPSPlayerState* TPSPlayerState;

	void PollInitPlayerState();

	//控制的龙
	UPROPERTY(ReplicatedUsing = OnRep_MountingDragon, VisibleAnywhere)
	ADragon* RidingDragon = nullptr;

	//骑乘的龙
	UPROPERTY(VisibleAnywhere)
	ADragon* MountingDragon = nullptr;

	UFUNCTION()
	void OnRep_MountingDragon();
public:

	UPROPERTY()
	class ATPSPlayerController* TPSController;

	//只在PossessedBy修改保证不会失效
	UPROPERTY()
	ATPSPlayerController* SavedTPSController;

	UPROPERTY(VisibleAnywhere)
	class AFlag* CarringFlag;

	void Elim();

	void HitEnemy();

	void StopControlling();

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	ANewShooterCharacter();

	void OnGroundAimOffset(float DeltaTime);
	void OnBoardAimOffset(float DeltaTime);

	void OnBoardTurningInPlace(float DeltaTime);

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetOverlappingDragon(ADragon* Dragon);
	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsElimmed();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	AWeapon* GetEquippedWeapon();
	// FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	void PlayFireMontage(bool bAiming);

	void PlayReloadMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRide(ADragon* DragontToMount);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnBoard(ADragon* DragonToMount);

	UFUNCTION(Server, Reliable)
	void ServerDismount();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDismount();

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE ATPSPlayerController* GetSavedTPSPlayerController() const { return SavedTPSController; }
	void ServerRideDragon();
	void ServerOnBoardDragon();
	void SetMeshNoCollision();

	UFUNCTION(Server, Reliable)
	void ServerSetSprint(bool Acc);


};


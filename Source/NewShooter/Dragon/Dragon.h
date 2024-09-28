// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Components/FlightComponent.h"
#include "Dragon.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class NEWSHOOTER_API ADragon : public ACharacter
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UFlightComponent* FlightComponent;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FastFlightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DismountAction;


	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> AbilityClass;

	UPROPERTY(Replicated)
	AWeapon* EquippedAbility;

	FVector TraceEndLocation;

	class ADragonAIController* DefaultAIController;

public:
	bool bFireButtonPressed;
	// Sets default values for this character's properties
	ADragon();
	virtual void PawnClientRestart() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere)
	class ANewShooterCharacter* PrimaryRidingCharacter;

	UPROPERTY(VisibleAnywhere)
	class ANewShooterCharacter* SecondaryRidingCharacter;

	void SummonedTo(const FVector& Location);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

public:	
	float CalculateAngleBetweenCameraAndCharacter();
	void ShowCrosshairAndEnableFiring(bool bShow);
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE float GetLeanX() { return FlightComponent->LeanX; }
	FORCEINLINE float GetLeanY() { return FlightComponent->LeanY; }
	FORCEINLINE bool GetFastFlight() { return FlightComponent->FastFlight; }

	void SetRider(ANewShooterCharacter* RiderCharacterToRide);
	void AttachCharacterToSocket(const FName& SocketName, ANewShooterCharacter* RiderCharacterToRide);
	void SetMount(ANewShooterCharacter* RiderCharacterToRide);

	virtual void PostInitializeComponents() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_Controller() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void TraceAndFire(class AFireThrower* FireAbility);
	void RemoveInputMappingContext();
	void DismountButtonPressed();
	void LocalClientDismount();
private:
	bool bDragonInProperAngle = true;

	void Move(const FInputActionValue& Value);
	void MoveButtonReleased();
	void TraceUnderCrosshaires();

	void CalculateCrosshairWorldPosition(FVector2D& ViewportSize, bool& bScreenToWorld, FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection);

	void SetCrosshairPosition();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void FastFlight(const FInputActionValue& Value);
	FVector GetMuzzelLocation();

	UFUNCTION(Server, Reliable)
	void ServerDismount();

	UFUNCTION()
	void ControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	void FlyAwayTimerFinished();

	FTimerHandle FlyAwayTimer;
};

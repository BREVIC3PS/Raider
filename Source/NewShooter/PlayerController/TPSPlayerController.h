// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "../Interfaces/TeamInterface.h"
#include "TPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NEWSHOOTER_API ATPSPlayerController : public APlayerController, public ITeamInterface
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDReadyState(bool ReadyOrNot);
	void SetHUDCrosshair();
	void SetHUDDragonCrosshair(bool bShow);
	void ShowHUDEnemyHit(bool bShow);
	void ShowHUDDragonEnemyHit(bool bShow);
	void SetHUDDragonCrosshairPosition(FVector2D NewPos);
	void SetHUDTeamScore(TArray<int32>& TeamScores);
	void SetTeamRingPassedText(TArray<int32>& TeamPassRingCount);
	void SetTeamId(const ETeamID& TeamID) override;
	ETeamID GetTeamId() override;
	void ReadyButtonPressed();
	void HitEnemy();
	UFUNCTION(Client,Unreliable)
	void ClientHitEnemy();

	UFUNCTION(Server,Reliable)
	void ServerPlayerReady();

	virtual void SetupInputComponent() override;

	void SetHUDMatchStart();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE class ANewShooterCharacter* GetOwningCharacter() { return OwningCharacter; }

	UFUNCTION(Server, Reliable)
	void ServerHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit, float Damage);
protected:
	virtual void BeginPlay() override;

	virtual void OnRep_Pawn() override;
private:
	UPROPERTY()
	class ACharacterHUD* HUD;
	
	/**Mappingcontext*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReadyAction;
	
	UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_NewShooterCharacter)
	ANewShooterCharacter* NewShooterCharacter;

	ANewShooterCharacter* OwningCharacter;

	UFUNCTION()
	void OnRep_NewShooterCharacter();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_DragonCharacter)
	class ADragon* DragonCharacter;

	ADragon* OwningDragon;

	UFUNCTION()
	void OnRep_DragonCharacter();

	FTimerHandle HUDChangeColorTimer;
	void HUDChangeColorFinished();
};

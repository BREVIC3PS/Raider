// Copyright Epic Games, Inc. All Rights Reserved.

#include "NewShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Weapon/Weapon.h"
#include "Components/CombatComponent.h"
#include "Components/FlightComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include <Net/UnrealNetwork.h>
#include "NewShooter/PlayerController/TPSPlayerController.h"
#include "GameMode/TPSGameMode.h"
#include "TimerManager.h"
#include "NewShooter/PlayerState/TPSPlayerState.h"
#include "NewShooter/NewShooter.h"
#include "NewShooter/Widgets/HealthBarWidgetComponent.h"
#include "NewShooter/Vehicles/FlyingPlatform.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Dragon/Dragon.h"
#include "Pickup/Flag.h"
#include <Kismet/GameplayStatics.h>

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANewShooterCharacter

ANewShooterCharacter::ANewShooterCharacter()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	Combat->Character = this;

	/*FlightComponent = CreateDefaultSubobject<UFlightComponent>(TEXT("FlightComponent"));
	FlightComponent->SetIsReplicated(true);
	FlightComponent->Character = this;*/

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	HealthBarWidget = CreateDefaultSubobject<UHealthBarWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	SetReplicateMovement(true);
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANewShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANewShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ANewShooterCharacter, OverlappingDragon, COND_OwnerOnly);
	DOREPLIFETIME(ANewShooterCharacter, Health);
	DOREPLIFETIME(ANewShooterCharacter, CharacterState);
	DOREPLIFETIME_CONDITION(ANewShooterCharacter, RidingDragon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ANewShooterCharacter, ReplicatedControlRotationYaw, COND_SimulatedOnly);
}

void ANewShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	/*if (FlightComponent)
	{
		FlightComponent->Character = this;
	}*/
}



void ANewShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
}

void ANewShooterCharacter::SetOverlappingDragon(ADragon* Dragon)
{
	OverlappingDragon = Dragon;
}

bool ANewShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ANewShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

bool ANewShooterCharacter::IsElimmed()
{
	return bElimmed;
}

AWeapon* ANewShooterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ANewShooterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

void ANewShooterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ANewShooterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		default:
			return;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//服务端和客户端
void ANewShooterCharacter::MulticastRide_Implementation(ADragon* DragonToMount)
{
	if (DragonToMount)
	{
		RidingDragon = DragonToMount;
	}
	SetMeshNoCollision();
	if (RidingDragon)
		RidingDragon->SetRider(this);
}


void ANewShooterCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

void ANewShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UpdateHUDHealth();
	PollInitPlayerState();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ANewShooterCharacter::ReceiveDamage);
	}
	HideHealthBarIfIsEnemyOrSelf();

	//SpawnFlyingPlatform();
	//FlightComponent->SetFlyMode();
}

void ANewShooterCharacter::HideHealthBarIfIsEnemyOrSelf()
{
	if (HasAuthority())return;
	if (IsLocallyControlled())
	{

		HealthBarWidget->SetVisibility(false);
		return;
	}
	ATPSPlayerController* LocalPlayerController = Cast< ATPSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (LocalPlayerController)
	{
		ETeamID MyId = LocalPlayerController->GetTeamId();
		if (MyId != ETeamID::NoTeamId)
		{
			ATPSPlayerState* OtherPlayerState = GetPlayerState<ATPSPlayerState>();
			if (OtherPlayerState)
			{
				if (OtherPlayerState->GetTeamId() != MyId)
				{
					HealthBarWidget->SetVisibility(false);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Other PlayerState is empty!"));
			}
		}
	}
}
//客户端执行
void ANewShooterCharacter::UpdateHUDHealth()
{
	if (HasAuthority())return;
	TPSController = TPSController == nullptr ? Cast<ATPSPlayerController>(Controller) : TPSController;
	if (TPSController)
	{
		TPSController->SetHUDHealth(Health, MaxHealth);
	}
	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Health / MaxHealth);
	}
}

//服务端调用
void ANewShooterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	//判断是不是友军火力，友军则不造成伤害
	ATPSPlayerController* DamageCauserController = Cast<ATPSPlayerController>(InstigatorController);
	if (!TPSPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReceiveDamage: PlayerState is empty! Intializing PlayerState..."));
		TPSPlayerState = GetPlayerState<ATPSPlayerState>();
	}

	if (DamageCauserController && TPSPlayerState)
	{
		ATPSPlayerState* DamageCauserPlayerState = DamageCauserController->GetPlayerState<ATPSPlayerState>();
		if (DamageCauserPlayerState->GetTeamId() == TPSPlayerState->GetTeamId())
		{
			return;
		}
		if (HasAuthority())
		{
			DamageCauserController->ClientHitEnemy();
		}
		else DamageCauserController->HitEnemy();
		Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

		if (Health == 0.0f)
		{
			ATPSGameMode* GameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
			if (GameMode)
			{
				// TPSController在UpdateHUDHealth()虽然设置过一遍，但不能保证不是Nullptr
				TPSController = TPSController == nullptr ? Cast<ATPSPlayerController>(Controller) : TPSController;
				ATPSPlayerController* AttckerController = Cast<ATPSPlayerController>(InstigatorController);
				GameMode->PlayerEliminated(this, TPSController, AttckerController);
			}
		}
	}

}

void ANewShooterCharacter::PollInitPlayerState()
{
	if (!TPSPlayerState)
	{
		TPSPlayerState = GetPlayerState<ATPSPlayerState>();
		if (TPSPlayerState)
		{
			TPSPlayerState->AddToScore(0.f);
		}
	}
}

void ANewShooterCharacter::OnRep_MountingDragon()
{
	//GetCharacterMovement()->StopMovementImmediately();
	//SetMeshNoCollision();
}

void ANewShooterCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	if (CarringFlag)
	{
		CarringFlag->Dropped();
	}
	if (CharacterState == ECharacterState::ECS_Riding || CharacterState == ECharacterState::ECS_Mounting)
	{
		ServerDismount();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ANewShooterCharacter::ElimTimerFinished,
		ElimDelay
	);

}

void ANewShooterCharacter::HitEnemy()
{
	TPSController = TPSController == nullptr ? Cast<ATPSPlayerController>(Controller) : TPSController;
	if (TPSController)
	{
		TPSController->HitEnemy();
	}
}

void ANewShooterCharacter::StopControlling()
{
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (TPSController)
	{
		DisableInput(TPSController);
	}
	SetMeshNoCollision();
}

void ANewShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ATPSPlayerController* PlayerController = Cast<ATPSPlayerController>(NewController);
	if (PlayerController)
	{
		TPSController = PlayerController;
		SavedTPSController = PlayerController;
	}
}

//客户端调用
void ANewShooterCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	StopControlling();
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	if (TPSController)
	{
		TPSController->SetHUDAmmo(0);
		//TPSController->SetHUDHealth(0, MaxHealth);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANewShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ANewShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ANewShooterCharacter::MoveButtonReleased);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::Look);

		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::EquipButtonPressed);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ANewShooterCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ANewShooterCharacter::CrouchButtonPressed);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::Aim);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(StopFireAction, ETriggerEvent::Completed, this, &ANewShooterCharacter::FireButtonReleased);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(CallDragonAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::CallDragonPressed);

		EnhancedInputComponent->BindAction(AccelerationAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::AccButtonPressed);
		EnhancedInputComponent->BindAction(RotateYawAction, ETriggerEvent::Triggered, this, &ANewShooterCharacter::RotateYawButtonPressed);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANewShooterCharacter::OnGroundAimOffset(float DeltaTime)
{
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy) // 如果是表示本地控制的角色，则rotate rootbone
	{
		AimOffset(DeltaTime);
		//LastCameraLocation = UKismetMathLibrary::FInterpTo()
	}
	else
	{
		// 先不研究AimOffset 了 
		//TODO: 为proxy 添加AimOffset
		SimProxiesTurn();
	}
}

void ANewShooterCharacter::OnBoardAimOffset(float DeltaTime)
{
	if (MountingDragon && MountingDragon->GetFastFlight() == true)
	{
		bUseControllerRotationYaw = false;
	}
	else
	{
		bUseControllerRotationYaw = true;
		SetActorRotation(FRotator(0.f, ReplicatedControlRotationYaw, 0.f));
	}
	
	bRotateRootBone = false;//设置为false，因为OnBorad函数可能发生在Tick之前导致bRotateRootBone为true
	//Calculate AO_Yaw
	if (IsLocallyControlled() || HasAuthority())
	{
		ReplicatedControlRotationYaw = GetBaseAimRotation().Yaw;
	}
	FRotator CurrentAimRotation = FRotator(0.f, ReplicatedControlRotationYaw, 0.f);
	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, FRotator(0, GetActorRotation().Yaw, 0));
	AO_Yaw = DeltaAimRotation.Yaw;
	/*if ((AO_Yaw > 90 || AO_Yaw < -90))
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		return OnBoardTurningInPlace(DeltaTime);
	}*/
	//Calculate AO_Pitch
	if (!IsLocallyControlled() && !HasAuthority())
	{
		float RemotePitch = RemoteViewPitch;
		// map pitch from [192, 256) to [-90,0) bcs UE mapped our AO_Pitch into a short varible
		if (RemotePitch > 90.f)
		{
			FVector2D InRange(192.f, 256.f);
			FVector2D OutRange(-90.f, 0.f);
			RemotePitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, RemotePitch);
		}
		FRotator CurrentAimRotationPitch = FRotator(RemotePitch, 0.f, 0.f);
		FRotator DeltaAimRotationPitch = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotationPitch, FRotator(GetActorRotation().Pitch, 0, 0));
		AO_Pitch = DeltaAimRotationPitch.Pitch;
	}
	else
	{
		FRotator CurrentAimRotationPitch = FRotator(GetBaseAimRotation().Pitch, 0.f, 0.f);
		FRotator DeltaAimRotationPitch = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotationPitch, FRotator(GetActorRotation().Pitch, 0, 0));
		AO_Pitch = DeltaAimRotationPitch.Pitch;
	}

}

void ANewShooterCharacter::OnBoardTurningInPlace(float DeltaTime)
{
	//if(HasAuthority())UE_LOG(LogTemp, Warning, TEXT("Server:%f"), AO_Yaw);
	//if (!HasAuthority())return;
	//InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, ReplicatedControlRotationYaw, DeltaTime, 5.f);
	SetActorRotation(FRotator(GetActorRotation().Pitch, ReplicatedControlRotationYaw, GetActorRotation().Roll));
	//AO_Yaw = InterpAO_Yaw;
	/*UE_LOG(LogTemp, Warning, TEXT("InterpAO_Yaw:%f"), InterpAO_Yaw);
	UE_LOG(LogTemp, Warning, TEXT("GetControlRotation().Yaw:%f"), ReplicatedControlRotationYaw);*/
	if (FMath::Abs(AO_Yaw) < 5.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ANewShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsElimmed() && GetActorLocation().Z < -3000 && HasAuthority())
	{
		ATPSGameMode* GameMode = Cast<ATPSGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)GameMode->PlayerEliminated(this,nullptr,nullptr);
		return;
	}
	if (CharacterState == ECharacterState::ECS_OnFoot)
	{
		OnGroundAimOffset(DeltaTime);
	}
	else if (CharacterState == ECharacterState::ECS_Mounting)
	{
		OnBoardAimOffset(DeltaTime);
	}
	/*if (CharacterState == ECharacterState::ECS_OnFoot )
	{
		UE_LOG(LogTemp, Warning, TEXT("EMovementMode:%d"),GetCharacterMovement()->MovementMode);
	}*/

}

void ANewShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		//OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		//LastWeapon->ShowPickupWidget(false);
	}
}

void ANewShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat && OverlappingWeapon)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CharacterState == ECharacterState::ECS_Mounting && MountingDragon)
		{
			ServerDismount();
		}


		else if (OverlappingDragon)
		{
			if (!OverlappingDragon->PrimaryRidingCharacter)
			{
				ServerRideDragon();
			}
			else if (!OverlappingDragon->SecondaryRidingCharacter)
			{
				ServerOnBoardDragon();
			}
		}
	}
}



void ANewShooterCharacter::ServerDismount_Implementation()
{
	if (RidingDragon && CharacterState == ECharacterState::ECS_Riding)
	{
		RidingDragon->ServerDismount_Implementation();//Detach and call MulticastDismount
	}
	else if (MountingDragon && CharacterState == ECharacterState::ECS_Mounting)
	{
		MulticastDismount();
	}
}

//服务端调用
void ANewShooterCharacter::ServerRideDragon()
{
	RidingDragon = OverlappingDragon; //调用OnRep_MountingDragon，目前为空
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	TPSController = TPSController == nullptr ? Cast<ATPSPlayerController>(Controller) : TPSController;
	if (TPSController)
	{
		TPSController->Possess(RidingDragon);
	}
	CharacterState = ECharacterState::ECS_Riding;//调用OnRep_CharacterState函数，目前为空
	MulticastRide(RidingDragon);

}
void ANewShooterCharacter::ServerOnBoardDragon()
{
	MountingDragon = OverlappingDragon;
	MulticastOnBoard(MountingDragon);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	CharacterState = ECharacterState::ECS_Mounting;
}

void ANewShooterCharacter::MulticastOnBoard_Implementation(ADragon* DragonToMount)
{
	MountingDragon = DragonToMount;
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	SetMeshNoCollision();
	if (MountingDragon)
		MountingDragon->SetMount(this);
	//bUseControllerRotationYaw = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bRotateRootBone = false;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustments = 2;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustmentsLargeCorrection = 2;
	GetCharacterMovement()->NetworkLargeClientCorrectionDistance = 50;
}
void ANewShooterCharacter::MulticastDismount_Implementation()
{
	if (RidingDragon && RidingDragon->PrimaryRidingCharacter == this)
	{
		//RidingDragon->GetCharacterMovement()->StopMovementImmediately();
		if (IsLocallyControlled())
		{
			RidingDragon->LocalClientDismount();
		}
		RidingDragon->PrimaryRidingCharacter = nullptr;
		RidingDragon = nullptr;
	}
	if (MountingDragon && MountingDragon->SecondaryRidingCharacter == this)
	{
		MountingDragon->SecondaryRidingCharacter = nullptr;
		MountingDragon = nullptr;
	}
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GetRootComponent()->DetachFromComponent(DetachRules);
	SetActorLocation(GetActorLocation() + FVector::UpVector * 100.f);
	SetActorRotation(FRotator(0, 0, 0));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	CharacterState = ECharacterState::ECS_OnFoot;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustments = 0.1;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustmentsLargeCorrection = 0.05;
	GetCharacterMovement()->NetworkLargeClientCorrectionDistance = 15;

}

void ANewShooterCharacter::SetMeshNoCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ANewShooterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ANewShooterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 6.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ANewShooterCharacter::ElimTimerFinished()
{
	ATPSGameMode* GameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
	if (GameMode)
	{
		GameMode->RequestRespawn(this, Controller);
	}
}

//客户端+服务端调用
void ANewShooterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

void ANewShooterCharacter::Jump()
{

	/*if (FlightComponent && GetCharacterMovement()->IsFalling())
	{
		if (GetVelocity().Length() > FlightComponent->FallingThreshold)
		{
			if (HasAuthority())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);

				SpawnFlyingPlatform();
				FlightComponent->SetFlyMode();

			}
		}
	}
	else*/
	{
		Super::Jump();
	}
}

void ANewShooterCharacter::SpawnFlyingPlatform()
{
	//const USkeletalMeshSocket* PlatformSocket = GetMesh()->GetSocketByName(FName("PlatformSocket"));
	//if (PlatformSocket)
	//{
	//	FTransform SocketTransform = PlatformSocket->GetSocketTransform(GetMesh());
	//	// From muzzle flash socket to hit location from TraceUnderCrosshairs
	//	FRotator TargetRotation = GetActorRotation();
	//	if (PlatformClass)
	//	{
	//		FActorSpawnParameters SpawnParams;
	//		SpawnParams.Owner = this;
	//		SpawnParams.Instigator = this;
	//		UWorld* World = GetWorld();
	//		if (World)
	//		{
	//			Platform = GetWorld()->SpawnActor<AFlyingPlatform>(
	//				PlatformClass,
	//				SocketTransform.GetLocation(),
	//				TargetRotation,
	//				SpawnParams
	//			);
	//		}
	//	}
	//	if (Platform)
	//	{
	//		PlatformSocket->AttachActor(Platform, GetMesh());
	//	}

	//}
}

void ANewShooterCharacter::OnRep_CharacterState()
{
	bRotateRootBone = false;
}

void ANewShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	/*if (GetCharacterMovement()->IsFlying())
	{
		if (FlightComponent)
		{
			FlightComponent->FlyControl(MovementVector);
		}
	}
	else */
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANewShooterCharacter::MoveButtonReleased()
{
	/*if (FlightComponent && GetCharacterMovement()->IsFlying())
	{
		FlightComponent->MoveStop();
	}*/
}

void ANewShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANewShooterCharacter::AccButtonPressed(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::ECS_Mounting)return;
	bool Acc = Value.Get<bool>();
	SetCharacterSprinting(Acc);
	ServerSetSprint(Acc);

}

void ANewShooterCharacter::ServerSetSprint_Implementation(bool Acc)
{
	SetCharacterSprinting(Acc);
}

void ANewShooterCharacter::SetCharacterSprinting(bool Acc)
{
	if (Acc)
	{
		CharacterState = ECharacterState::ECS_Sprinting;
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		// Configure character movement
		GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	}
	else
	{
		CharacterState = ECharacterState::ECS_OnFoot;
		if (Combat && Combat->EquippedWeapon)
			Combat->SetCharacterEquipped();
	}
	if (Combat)GetCharacterMovement()->MaxWalkSpeed = Acc ? SprintSpeed : Combat->BaseWalkSpeed;
}


void ANewShooterCharacter::RotateYawButtonPressed(const FInputActionValue& Value)
{
	/*if (FlightComponent)
	{
		FlightComponent->RotateYaw(Value.Get<float>());
	}*/
}

void ANewShooterCharacter::EquipButtonPressed()
{
	if (Combat && OverlappingWeapon)
	{
		ServerEquipButtonPressed();
	}
	else if (OverlappingDragon || CharacterState == ECharacterState::ECS_Mounting)
	{
		ServerEquipButtonPressed();
	}
}

void ANewShooterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}


void ANewShooterCharacter::FireButtonPressed()
{
	bool bCanFire = Combat && TurningInPlace == ETurningInPlace::ETIP_NotTurning && (CharacterState == ECharacterState::ECS_OnFoot || CharacterState == ECharacterState::ECS_Mounting);
	if (bCanFire)
	{
		Combat->FireButtonPressed(true);
	}
}

void ANewShooterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ANewShooterCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void ANewShooterCharacter::CallDragonPressed()
{
	if (CharacterState == ECharacterState::ECS_Riding && CharacterState == ECharacterState::ECS_Mounting)return;
	ADragon* NearestDragon = FindNearestDragon(GetActorLocation());
	if (NearestDragon)
	{
		ServerSummonNearestDragon(NearestDragon);
	}
}

ADragon* ANewShooterCharacter::FindNearestDragon(const FVector& TargetLocation)
{
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADragon::StaticClass(), AllActors);

	float MinDistance = MAX_FLT;
	ADragon* NearestDragon = nullptr;

	for (AActor* Actor : AllActors)
	{
		ADragon* Dragon = Cast<ADragon>(Actor);
		if (Dragon && !Dragon->PrimaryRidingCharacter && !Dragon->SecondaryRidingCharacter)
		{
			float Distance = FVector::Distance(Dragon->GetActorLocation(), TargetLocation);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestDragon = Dragon;
			}
		}
	}

	return NearestDragon;
}

void ANewShooterCharacter::ServerSummonNearestDragon_Implementation(ADragon* NearestDragon)
{
	if (NearestDragon)
	{
		NearestDragon->SummonedTo(GetActorLocation());
	}
}

void ANewShooterCharacter::Aim()
{
	if (Combat)
	{
		if (!IsAiming())
			Combat->SetAiming(true);
		else Combat->SetAiming(false);
	}
}

void ANewShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);

		bUseControllerRotationYaw = true;
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();


}

void ANewShooterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270,360) to [-90,0) bcs UE mapped our AO_Pitch into a short varible
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

//非本地角色不旋转根骨骼
void ANewShooterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	CalculateAO_Pitch();
	bRotateRootBone = false;
}

void ANewShooterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

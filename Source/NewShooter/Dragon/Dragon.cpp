// Fill out your copyright notice in the Description page of Project Settings.


#include "Dragon.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "../NewShooterCharacter.h"
#include "../Components/FlightComponent.h"
#include "Components/SphereComponent.h"
#include "../NewShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "../PlayerController/TPSPlayerController.h"
#include "../Weapon/Weapon.h"
#include <Net/UnrealNetwork.h>
#include "../Weapon/FireThrower.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "../AIController/DragonAIController.h"
// Sets default values
ADragon::ADragon()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 1000.f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(500.f, 500.0f, 500.f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	//GetCharacterMovement()->JumpZVelocity = 700.f;
	//GetCharacterMovement()->AirControl = 0.35f;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
	//GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	//GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	//GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxFlySpeed = 6000;
	GetCharacterMovement()->BrakingDecelerationFlying = 3000;
	GetCharacterMovement()->NavAgentProps.bCanFly = true;
	GetCharacterMovement()->DefaultLandMovementMode = EMovementMode::MOVE_Flying;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetReplicateMovement(true);
	bReplicates = true;
	bAlwaysRelevant = true;
	FlightComponent = CreateDefaultSubobject<UFlightComponent>(TEXT("FlightComponent"));
	FlightComponent->SetIsReplicated(true);
	FlightComponent->Character = this;
}
//客户端调用
void ADragon::PawnClientRestart()
{
	Super::PawnClientRestart();
	FlightComponent->SetFastFly(false);
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 1);
		}
	}
}

void ADragon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADragon, EquippedAbility);
}

//服务端首次调用，之后客户端调用
void ADragon::SetRider(ANewShooterCharacter* RiderCharacterToRide)
{
	PrimaryRidingCharacter = RiderCharacterToRide;
	FName SocketName = FName("RiderSocket");
	AttachCharacterToSocket(SocketName, RiderCharacterToRide);
}


void ADragon::SetMount(ANewShooterCharacter* RiderCharacterToRide)
{
	SecondaryRidingCharacter = RiderCharacterToRide;
	FName SocketName = FName("RiderSocket2");
	AttachCharacterToSocket(SocketName, RiderCharacterToRide);
}

void ADragon::AttachCharacterToSocket(const FName& SocketName, ANewShooterCharacter* RiderCharacterToRide)
{
	const USkeletalMeshSocket* RiderSocket = GetMesh()->GetSocketByName(SocketName);
	if (RiderSocket)
	{
		RiderSocket->AttachActor(RiderCharacterToRide, GetMesh());
	}
}

void ADragon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (FlightComponent)
	{
		FlightComponent->Character = this;
	}
}
//服务器调用
void ADragon::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GetCharacterMovement()->StopActiveMovement();
	if (Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("NewController: %s"), *NewController->GetName());
		ATPSPlayerController* TPSPlayerController  = Cast<ATPSPlayerController>(NewController);
		if (TPSPlayerController)
		{
			FlightComponent->TPSController = Cast<ATPSPlayerController>(NewController);
			GetWorldTimerManager().ClearTimer(FlyAwayTimer);
		}
		else
		{
			ADragonAIController* DragonAIController = Cast<ADragonAIController>(NewController);
			if (DragonAIController)
			{
				GetCharacterMovement()->StopMovementImmediately();
				DefaultAIController = DragonAIController;

			}
		}
	}
}

void ADragon::OnRep_Controller()
{
	Super::OnRep_Controller();
	FlightComponent->TPSController = Cast<ATPSPlayerController>(Controller);
	GetCharacterMovement()->StopActiveMovement();
}

void ADragon::SummonedTo(const FVector& Location)
{
	if (Controller == DefaultAIController)
	{
		DefaultAIController->MoveToLocation(Location, -1, false, false);
	}
}

// Called when the game starts or when spawned
void ADragon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ADragon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ADragon::OnSphereEndOverlap);
		FVector SpawnLocation = GetMuzzelLocation();
		AWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(AbilityClass, SpawnLocation, GetActorRotation());
		const USkeletalMeshSocket* FireSocket = GetMesh()->GetSocketByName(FName("FireSocket"));
		if (FireSocket)
		{
			SpawnedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true), FName("FireSocket"));
			//FireSocket->AttachActor(SpawnedWeapon, GetMesh());
		}
		EquippedAbility = SpawnedWeapon;
		EquippedAbility->SetOwner(this);

		ReceiveControllerChangedDelegate.AddDynamic(this, &ADragon::ControllerChanged);
	}
	
}
//服务端调用
void ADragon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ANewShooterCharacter* NewShooterCharacter = Cast<ANewShooterCharacter>(OtherActor);
	if (NewShooterCharacter)
	{
		NewShooterCharacter->SetOverlappingDragon(this);
	}
}

void ADragon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ANewShooterCharacter* NewShooterCharacter = Cast<ANewShooterCharacter>(OtherActor);
	if (NewShooterCharacter)
	{
		NewShooterCharacter->SetOverlappingDragon(nullptr);
	}
}

float ADragon::CalculateAngleBetweenCameraAndCharacter()
{
	// 获取角色的前向向量
	FVector CharacterForward = GetActorForwardVector();
	// 获取瞄准方向向量
	FVector AimDirection;
	ATPSPlayerController* PlayerController = Cast<ATPSPlayerController>(GetController());
	if (PlayerController)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
		AimDirection = CameraRotation.Vector();
	}
	// 计算夹角
	return FMath::RadiansToDegrees(acosf(FVector::DotProduct(CharacterForward, AimDirection)));
}

void ADragon::ShowCrosshairAndEnableFiring(bool bShow)
{
	if (!bShow)
	{
		ATPSPlayerController* PlayerController = Cast<ATPSPlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->SetHUDDragonCrosshair(false);
		}
		FireButtonReleased();
		bDragonInProperAngle = false;
	}
	else
	{
		ATPSPlayerController* PlayerController = Cast<ATPSPlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->SetHUDDragonCrosshair(true);
		}
		TraceUnderCrosshaires();
		SetCrosshairPosition();
		bDragonInProperAngle = true;
	}
}

// Called every frame
void ADragon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsLocallyControlled()&&!HasAuthority())
	{
		
		float Angle = CalculateAngleBetweenCameraAndCharacter();
		// 检查夹角是否超过100度
		ShowCrosshairAndEnableFiring(Angle < 100.f);
	}
}

// Called to bind functionality to input
void ADragon::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 1);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {


		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADragon::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ADragon::MoveButtonReleased);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADragon::Look);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ADragon::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ADragon::FireButtonReleased);

		EnhancedInputComponent->BindAction(FastFlightAction, ETriggerEvent::Triggered, this, &ADragon::FastFlight);
		EnhancedInputComponent->BindAction(FastFlightAction, ETriggerEvent::Completed, this, &ADragon::FastFlight);

		EnhancedInputComponent->BindAction(DismountAction, ETriggerEvent::Triggered, this, &ADragon::DismountButtonPressed);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ADragon::Move(const FInputActionValue& Value)
{
	FVector MovementVector = Value.Get<FVector>();
	if (GetCharacterMovement()->IsFlying())
	{
		if (FlightComponent)
		{
			FlightComponent->FlyControl(MovementVector);
		}
	}
}

void ADragon::MoveButtonReleased()
{
}

void ADragon::Look(const FInputActionValue& Value)
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

void ADragon::FireButtonPressed()
{
	if (bDragonInProperAngle)
	{
		bFireButtonPressed = true;
		if (EquippedAbility && !FlightComponent->FastFlight)
		{
			AFireThrower* FireAbility = Cast<AFireThrower>(EquippedAbility);
			if (FireAbility)
			{
				TraceAndFire(FireAbility);
			}
		}
	}
}

void ADragon::TraceAndFire(AFireThrower* FireAbility)
{
	if (bFireButtonPressed)
	{
		FireAbility->Fire(TraceEndLocation);
	}
	else {
		if (FireAbility->Activated)FireAbility->StopFire();
	}
}

void ADragon::FireButtonReleased()
{
	bFireButtonPressed = false;
	AFireThrower* FireAbility = Cast<AFireThrower>(EquippedAbility);
	if (FireAbility)
	{
		if(FireAbility->Activated)FireAbility->StopFire();
	}
}

void ADragon::FastFlight(const FInputActionValue& Value)
{
	bool bFastfly = Value.Get<bool>();
	FlightComponent->SetFastFly(bFastfly);
}

void ADragon::RemoveInputMappingContext()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}
}

void ADragon::DismountButtonPressed()
{

	if (PrimaryRidingCharacter)
	{
		LocalClientDismount();
		ServerDismount();
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void ADragon::LocalClientDismount()
{
	if (IsLocallyControlled())
	{
		RemoveInputMappingContext();
	}
}

void ADragon::TraceUnderCrosshaires()
{
	FVector Start;
	FVector2D ViewportSize;
	float TraceRange = 8000.f;
	bool bScreenToWorld = false;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	CalculateCrosshairWorldPosition(ViewportSize, bScreenToWorld, CrosshairWorldPosition, CrosshairWorldDirection);
	if (EquippedAbility)
	{
		Start = EquippedAbility->GetActorLocation();
		AFireThrower* FireAbility = Cast<AFireThrower>(EquippedAbility);
		if (FireAbility) {
			TraceRange = FireAbility->GetFireRange();
		}
	}
	else
	{
		if (bScreenToWorld)
		{
			float DistanceToCharacter = (GetActorLocation() - Start).Size();
			Start = CrosshairWorldPosition;
			Start += CrosshairWorldDirection * (DistanceToCharacter +  500.f);
		}
	}
	FVector End =  Start + CrosshairWorldDirection * TraceRange;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 忽略自身
	QueryParams.AddIgnoredActor(EquippedAbility); 
	QueryParams.AddIgnoredActor(PrimaryRidingCharacter); // 忽略自身
	QueryParams.AddIgnoredActor(SecondaryRidingCharacter); // 忽略自身
	FHitResult OutHit;
	GetWorld()->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity, // 旋转，这里我们使用默认的无旋转
		ECollisionChannel::ECC_Visibility, // 碰撞通道，可以根据需要选择不同的通道
		FCollisionShape::MakeSphere(100.f),
		QueryParams
	);
	TraceEndLocation = OutHit.ImpactPoint;
	if (!OutHit.bBlockingHit) {
		TraceEndLocation = End;
	}
	//DrawDebugLine(GetWorld(), Start, TraceEndLocation, FColor::Blue,false,2);
	//DrawDebugSphere(GetWorld(), TraceEndLocation, 30, 3, FColor::White);
}

void ADragon::CalculateCrosshairWorldPosition(FVector2D& ViewportSize, bool& bScreenToWorld, FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection)
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 5.f);
	bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		Cast<APlayerController>(Controller),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
}

void ADragon::SetCrosshairPosition()
{
	FVector2D ScreenCrosshairPosition;
	bool bWorldToScreen = UGameplayStatics::ProjectWorldToScreen(Cast<APlayerController>(Controller), TraceEndLocation, ScreenCrosshairPosition, false);
	if (bWorldToScreen)
	{
		ATPSPlayerController* PlayerController = Cast<ATPSPlayerController>(Controller);
		//ScreenCrosshairPosition -= ViewportSize/2;
		PlayerController->SetHUDDragonCrosshairPosition(ScreenCrosshairPosition);
		
	}
}

FVector ADragon::GetMuzzelLocation()
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetMesh()->GetSocketByName(FName("FireSocket"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetMesh());
		return SocketTransform.GetLocation();
	}
	return FVector();
}

void ADragon::ControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	if (!NewController && !OldController->IsA(AAIController::StaticClass()))
	{
		if (DefaultAIController)
		{
			DefaultAIController->Possess(this);
			/*if (!PrimaryRidingCharacter && !SecondaryRidingCharacter)
			{
			}*/
			GetWorldTimerManager().SetTimer(FlyAwayTimer, this, &ADragon::FlyAwayTimerFinished, 3, false);
		}

	}
}

void ADragon::FlyAwayTimerFinished()
{
	if (Controller == DefaultAIController)
	{
		DefaultAIController->MoveToLocation(GetActorLocation() + FVector::UpVector * 10000.f,-1,false,false);
	}
}


void ADragon::ServerDismount_Implementation()
{
	if (Controller)
	{
		Controller->Possess(PrimaryRidingCharacter);
		PrimaryRidingCharacter->MulticastDismount();
	}
}



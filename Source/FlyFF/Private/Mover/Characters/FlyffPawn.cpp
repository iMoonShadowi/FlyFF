#include "Mover/Characters/FlyffPawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"

// Enhanced Input
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AFlyffPawn::AFlyffPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(Capsule);
	Capsule->InitCapsuleSize(34.f, 88.f);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.f;                  // 3rd-person
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SocketOffset = FVector(0.f, 0.f, 40.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->MaxSpeed = 600.f;

	// (Optional) init pointers
	IMC_Default = nullptr;
	IA_MoveForward = nullptr;
	IA_MoveRight = nullptr;

	// (Optional) init cached values (must exist in .h)
	MoveForwardValue = 0.f;
	MoveRightValue = 0.f;
}

void AFlyffPawn::BeginPlay()
{
	Super::BeginPlay();

	// Free the mouse for UI/clicks
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);

		// Add our Mapping Context
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (IMC_Default)
				{
					Subsys->AddMappingContext(IMC_Default, 0);
				}
			}
		}
	}
}

void AFlyffPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFlyffPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// REMOVE any old binds to IA_Move before adding these:
		if (IA_MoveForward)
		{
			EIC->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AFlyffPawn::MoveForward);
			EIC->BindAction(IA_MoveForward, ETriggerEvent::Completed, this, &AFlyffPawn::MoveForward);
		}
		if (IA_MoveRight)
		{
			EIC->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &AFlyffPawn::MoveRight);
			EIC->BindAction(IA_MoveRight, ETriggerEvent::Completed, this, &AFlyffPawn::MoveRight);
		}
	}
}

void AFlyffPawn::MoveForward(const FInputActionValue& Value)
{
	// Make sure these fields exist in your .h:
	// float MoveForwardValue; float MoveRightValue;
	MoveForwardValue = Value.Get<float>();

	// Move relative to CAMERA yaw (3rd-person feel)
	const FRotator CamRot = SpringArm->GetComponentRotation();
	const FRotator YawOnly(0.f, CamRot.Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);

	AddMovementInput(Forward, MoveForwardValue);

	// Face input direction (uses cached right value too)
	if (!FMath::IsNearlyZero(MoveForwardValue) || !FMath::IsNearlyZero(MoveRightValue))
	{
		const float YawDeg = FMath::Atan2(MoveRightValue, MoveForwardValue) * 180.f / PI + CamRot.Yaw;
		SetActorRotation(FRotator(0.f, YawDeg, 0.f));
	}
}

void AFlyffPawn::MoveRight(const FInputActionValue& Value)
{
	MoveRightValue = Value.Get<float>();

	const FRotator CamRot = SpringArm->GetComponentRotation();
	const FRotator YawOnly(0.f, CamRot.Yaw, 0.f);
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

	AddMovementInput(Right, MoveRightValue);

	if (!FMath::IsNearlyZero(MoveForwardValue) || !FMath::IsNearlyZero(MoveRightValue))
	{
		const float YawDeg = FMath::Atan2(MoveRightValue, MoveForwardValue) * 180.f / PI + CamRot.Yaw;
		SetActorRotation(FRotator(0.f, YawDeg, 0.f));
	}
}
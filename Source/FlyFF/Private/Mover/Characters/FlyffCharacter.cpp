// Source/FlyFF/Private/Mover/Characters/FlyffCharacter.cpp

#include "Mover/Characters/FlyffCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"   // SimpleMoveToLocation (click-to-move)
#include "NavigationSystem.h"                      // nav types (optional)

// ==============================
// Constructor
// ==============================
AFlyffCharacter::AFlyffCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Camera boom ---
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 450.f;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw   = false;
    SpringArm->bInheritRoll  = false;
    SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));

    // --- Camera ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // --- Tank controls (actor turns via controller yaw) ---
    bUseControllerRotationYaw = true;
    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = false;  // IMPORTANT for tank-turning
    Move->RotationRate   = FRotator(0.f, 720.f, 0.f);
    Move->MaxWalkSpeed   = 600.f;
    Move->JumpZVelocity  = 600.f;
    Move->AirControl     = 0.35f;
    Move->BrakingFrictionFactor = 1.5f;       // snappier stop/turn

    // --- Collision: capsule drives; mesh is visual only ---
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
        Capsule->SetGenerateOverlapEvents(false);
    }
    if (USkeletalMeshComponent* Sk = GetMesh())
    {
        Sk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Sk->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        Sk->SetGenerateOverlapEvents(false);
        Sk->SetSimulatePhysics(false);
    }

    // --- Smooth camera follow ---
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 12.f;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraRotationLagSpeed = 15.f;
}

// ==============================
// BeginPlay
// ==============================
void AFlyffCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // Keep mouse free for UI/clicks
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;

        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(Mode);
    }

    // prime yaw tracking for camera follow
    LastActorYaw = GetActorRotation().Yaw;
    bFirstTickYaw = false;
}

// ==============================
// Input bindings
// ==============================
void AFlyffCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // W/S forward/back (+ autorun via W double-tap)
    PlayerInputComponent->BindKey(EKeys::W, IE_Pressed,  this, &AFlyffCharacter::W_Pressed);
    PlayerInputComponent->BindKey(EKeys::W, IE_Released, this, &AFlyffCharacter::W_Released);
    PlayerInputComponent->BindKey(EKeys::S, IE_Pressed,  this, &AFlyffCharacter::S_Pressed);
    PlayerInputComponent->BindKey(EKeys::S, IE_Released, this, &AFlyffCharacter::S_Released);

    // A/D = turn (tank controls)
    PlayerInputComponent->BindKey(EKeys::A, IE_Pressed,  this, &AFlyffCharacter::A_Pressed);
    PlayerInputComponent->BindKey(EKeys::A, IE_Released, this, &AFlyffCharacter::A_Released);
    PlayerInputComponent->BindKey(EKeys::D, IE_Pressed,  this, &AFlyffCharacter::D_Pressed);
    PlayerInputComponent->BindKey(EKeys::D, IE_Released, this, &AFlyffCharacter::D_Released);

    // Jump
    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed,  this, &AFlyffCharacter::Space_Pressed);
    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &AFlyffCharacter::Space_Released);

    // Mouse: RMB hold = free-look, LMB click-to-move
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed,  this, &AFlyffCharacter::RMB_Pressed);
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AFlyffCharacter::RMB_Released);
    PlayerInputComponent->BindKey(EKeys::LeftMouseButton,  IE_Pressed,  this, &AFlyffCharacter::LMB_Pressed);
}

// ==============================
// Tick
// ==============================
void AFlyffCharacter::Tick(float dt)
{
    Super::Tick(dt);
    ApplyTurn(dt);
    ApplyMovement(dt);
    ApplyFreeLook(dt);
    UpdateCameraFollow(dt);   // <— camera tracks actor yaw changes (A/D) and forward movement
}

// ==============================
// Per-tick helpers
// ==============================

void AFlyffCharacter::ApplyTurn(float dt)
{
    if (!FMath::IsNearlyZero(TurnAxis))
        AddControllerYawInput(TurnAxis * CameraYawSpeedDegPerSec * dt);
}

void AFlyffCharacter::ApplyMovement(float /*dt*/)
{
    // autorun keeps you moving forward without W held
    const float Forward = (bAutoRun ? 1.f : 0.f) + ForwardAxis; // S can still push negative
    if (!FMath::IsNearlyZero(Forward))
        AddMovementInput(GetActorForwardVector(), FMath::Clamp(Forward, -1.f, 1.f));
}

void AFlyffCharacter::ApplyFreeLook(float dt)
{
    if (!bCameraFrozen) return;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        float DX=0.f, DY=0.f;
        PC->GetInputMouseDelta(DX, DY);

        FRotator R = SpringArm->GetComponentRotation();
        R.Yaw   += DX * CameraYawSpeedDegPerSec * dt * 0.5f;
        R.Pitch = FMath::Clamp(R.Pitch - DY * 80.f * dt, -80.f, 10.f);
        SpringArm->SetWorldRotation(R);
    }
}

// NEW: follow camera whenever actor yaw changes (A/D) or when moving forward
void AFlyffCharacter::UpdateCameraFollow(float dt)
{
    if (bCameraFrozen) return;

    const float ActorYaw = GetActorRotation().Yaw;

    if (bFirstTickYaw)
    {
        LastActorYaw = ActorYaw;
        bFirstTickYaw = false;
    }

    // detect turning by measuring actor yaw change
    const float YawChange = FMath::FindDeltaAngleDegrees(LastActorYaw, ActorYaw);
    const float TurnRateDegPerSec = (dt > KINDA_SMALL_NUMBER) ? (YawChange / dt) : 0.f;
    const bool  bIsTurning = FMath::Abs(TurnRateDegPerSec) > 1.f;

    float FollowSpeed = 0.f;
    if (bIsTurning)
    {
        FollowSpeed = CameraFollowTurnSpeedDegPerSec;    // fast follow during A/D turning
    }
    else if (bAutoRun || ForwardAxis > 0.f)
    {
        FollowSpeed = CameraAlignSpeedDegPerSec;         // gentle follow while moving forward
    }
    else
    {
        LastActorYaw = ActorYaw;
        return;
    }

    FRotator Cam = SpringArm->GetComponentRotation();
    const float CamDelta = FMath::FindDeltaAngleDegrees(Cam.Yaw, ActorYaw);
    if (!FMath::IsNearlyZero(CamDelta, 0.1f))
    {
        const float Step = FMath::Clamp(CamDelta, -FollowSpeed * dt, FollowSpeed * dt);
        Cam.Yaw += Step;
        SpringArm->SetWorldRotation(Cam);
    }

    LastActorYaw = ActorYaw;
}

// ==============================
// Key handlers
// ==============================

static float ClampAxis(float V){ return FMath::Clamp(V, -1.f, 1.f); }

void AFlyffCharacter::W_Pressed()
{
    const double Now = FPlatformTime::Seconds();
    if (LastWPressTime > 0 && (Now - LastWPressTime) <= DoubleTapSecs)
        bAutoRun = !bAutoRun; // toggle autorun on double-tap
    LastWPressTime = Now;

    ForwardAxis = ClampAxis(ForwardAxis + 1.f);
}
void AFlyffCharacter::W_Released() { ForwardAxis = ClampAxis(ForwardAxis - 1.f); }

void AFlyffCharacter::S_Pressed()
{
    bAutoRun = false; // cancel autorun if you press back
    ForwardAxis = ClampAxis(ForwardAxis - 1.f);
}
void AFlyffCharacter::S_Released(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }

void AFlyffCharacter::A_Pressed()  { TurnAxis = ClampAxis(TurnAxis - 1.f); }
void AFlyffCharacter::A_Released() { TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_Pressed()  { TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_Released() { TurnAxis = ClampAxis(TurnAxis - 1.f); }

void AFlyffCharacter::Space_Pressed(){ Jump(); }
void AFlyffCharacter::Space_Released(){ StopJumping(); }

void AFlyffCharacter::RMB_Pressed(){ bCameraFrozen = true; }
void AFlyffCharacter::RMB_Released(){ bCameraFrozen = false; }

void AFlyffCharacter::LMB_Pressed()
{
    if (bCameraFrozen) return; // ignore while freelook

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, Hit.ImpactPoint);
            bAutoRun = false; // click-move takes over
        }
    }
}

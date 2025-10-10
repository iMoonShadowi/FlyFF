#include "Mover/Characters/FlyffCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"   // SimpleMoveToLocation (optional LMB click-to-move)
#include "InputCoreTypes.h"

// ---------------- Constructor ----------------
AFlyffCharacter::AFlyffCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Spring arm (camera boom) ---
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = CamTune.DefaultArmLength;
    SpringArm->SetRelativeRotation(FRotator(CamTune.DefaultPitchDegrees, 0.f, 0.f));

    // Follow the controller's yaw (so camera stays behind while A/D turns)
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritYaw   = true;   // IMPORTANT: camera follows yaw
    SpringArm->bInheritPitch = false;  // keep fixed down-tilt from the arm
    SpringArm->bInheritRoll  = false;

    // Feel: position lag OK, rotation lag OFF for tight behind-the-back follow
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed   = CamTune.Lag;
    SpringArm->bEnableCameraRotationLag = false;

    // --- Camera ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false; // spring arm handles control rotation

    // --- Character movement: "tank" turning via controller yaw ---
    bUseControllerRotationYaw = true;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = false;
    Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    Move->AirControl  = 0.35f;
    Move->BrakingFrictionFactor = 1.5f;
    SetSpeedByState();

    // --- Collision & mesh offsets ---
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

        Sk->SetRelativeLocation(FVector(0.f, 0.f, MeshZOffset));
        FRotator R = Sk->GetRelativeRotation();
        R.Yaw = MeshYawOffsetDeg;
        Sk->SetRelativeRotation(R);
    }
}

// ---------------- BeginPlay ----------------
void AFlyffCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Expected flags for coupled rig
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Make sure the boom actually follows control rotation
    if (SpringArm) SpringArm->bUsePawnControlRotation = true;
    if (Camera)    Camera->bUsePawnControlRotation    = false;

    // Force view/control to this pawn
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bAutoManageActiveCameraTarget = true;
        if (Camera) Camera->Activate(true);
        PC->SetViewTarget(this);

        PC->bShowMouseCursor = false;
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        EnableInput(PC);
    }

    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

// ---------------- Input ----------------
void AFlyffCharacter::SetupPlayerInputComponent(UInputComponent* IC)
{
    Super::SetupPlayerInputComponent(IC);

    // WASD
    IC->BindKey(EKeys::W, IE_Pressed,  this, &AFlyffCharacter::W_P);
    IC->BindKey(EKeys::W, IE_Released, this, &AFlyffCharacter::W_R);
    IC->BindKey(EKeys::S, IE_Pressed,  this, &AFlyffCharacter::S_P);
    IC->BindKey(EKeys::S, IE_Released, this, &AFlyffCharacter::S_R);

    IC->BindKey(EKeys::A, IE_Pressed,  this, &AFlyffCharacter::A_P);
    IC->BindKey(EKeys::A, IE_Released, this, &AFlyffCharacter::A_R);
    IC->BindKey(EKeys::D, IE_Pressed,  this, &AFlyffCharacter::D_P);
    IC->BindKey(EKeys::D, IE_Released, this, &AFlyffCharacter::D_R);

    // Optional click-to-move + snap camera
    IC->BindKey(EKeys::LeftMouseButton,  IE_Pressed, this, &AFlyffCharacter::LMB_P);
    IC->BindKey(EKeys::MiddleMouseButton,IE_Pressed, this, &AFlyffCharacter::MMB_P);

    // Speed, autorun, walk toggle
    IC->BindKey(EKeys::LeftShift, IE_Pressed,  this, &AFlyffCharacter::Shift_P);
    IC->BindKey(EKeys::LeftShift, IE_Released, this, &AFlyffCharacter::Shift_R);
    IC->BindKey(EKeys::LeftAlt,   IE_Pressed,  this, &AFlyffCharacter::Alt_P);
    IC->BindKey(EKeys::LeftAlt,   IE_Released, this, &AFlyffCharacter::Alt_R);
    IC->BindKey(EKeys::F,         IE_Pressed,  this, &AFlyffCharacter::F_P);
    IC->BindKey(EKeys::X,         IE_Pressed,  this, &AFlyffCharacter::X_P); // NEW: toggle walk/run

    // Jump
    IC->BindKey(EKeys::SpaceBar, IE_Pressed,  this, &AFlyffCharacter::Space_P);
    IC->BindKey(EKeys::SpaceBar, IE_Released, this, &AFlyffCharacter::Space_R);
}

// ---------------- Tick ----------------
void AFlyffCharacter::Tick(float dt)
{
    Super::Tick(dt);
    TickMove(dt);
    TickCamera(dt);

    // quick overlay
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(7, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Speed=%.0f  Fwd=%.1f  Turn=%.1f  Auto=%d  Walk=%d Sprint=%d"),
            GetCharacterMovement()->MaxWalkSpeed, ForwardAxis, TurnAxis, bAutorun?1:0, bWalk?1:0, bSprint?1:0));
}

// ---------------- Movement ----------------
void AFlyffCharacter::TickMove(float dt)
{
    SetSpeedByState();

    // Turn with A/D (tank turning)
    if (!FMath::IsNearlyZero(TurnAxis))
        AddControllerYawInput(TurnAxis * MoveTune.TurnSpeedDeg * dt);

    // Move with W/S along actor forward (+ autorun)
    const float Forward = (bAutorun ? 1.f : 0.f) + ForwardAxis;
    if (!FMath::IsNearlyZero(Forward))
        AddMovementInput(GetActorForwardVector(), FMath::Clamp(Forward, -1.f, 1.f));
}

void AFlyffCharacter::SetSpeedByState()
{
    // Walk overrides Sprint; else Sprint > Run
    float Target = MoveTune.RunSpeed;
    if (bWalk)          Target = MoveTune.WalkSpeed;
    else if (bSprint)   Target = MoveTune.SprintSpeed;

    GetCharacterMovement()->MaxWalkSpeed = Target;
}

// ---------------- Camera ----------------
void AFlyffCharacter::TickCamera(float /*dt*/)
{
    if (!SpringArm) return;

    // Ensure we’re viewing THIS pawn/camera (useful if you swap pawns)
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (Camera && !Camera->IsActive()) Camera->Activate(true);
        if (PC->GetViewTarget() != this)   PC->SetViewTarget(this);
    }
}

// ---------------- Input handlers ----------------
// WASD
void AFlyffCharacter::W_P(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }
void AFlyffCharacter::W_R(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); }
void AFlyffCharacter::S_P(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); bAutorun=false; }
void AFlyffCharacter::S_R(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }

void AFlyffCharacter::A_P(){ TurnAxis = ClampAxis(TurnAxis - 1.f); }
void AFlyffCharacter::A_R(){ TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_P(){ TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_R(){ TurnAxis = ClampAxis(TurnAxis - 1.f); }

// Jump
void AFlyffCharacter::Space_P(){ Jump(); }
void AFlyffCharacter::Space_R(){ StopJumping(); }

// Snap camera behind (MMB)
void AFlyffCharacter::MMB_P()
{
    if (AController* C = GetController())
    {
        FRotator Ctrl = C->GetControlRotation();
        Ctrl.Yaw = GetActorRotation().Yaw;
        C->SetControlRotation(Ctrl);
    }
}

// Speed & autorun
void AFlyffCharacter::Shift_P(){ if (!bWalk) bSprint = true; } // sprint only if not in walk toggle
void AFlyffCharacter::Shift_R(){ bSprint = false; }
void AFlyffCharacter::Alt_P()  { bWalk   = true;  bSprint=false; } // hold-to-walk
void AFlyffCharacter::Alt_R()  { bWalk   = false; }                // release back to run
void AFlyffCharacter::F_P()    { bAutorun = !bAutorun; }

// Toggle walk/run (X)
void AFlyffCharacter::X_P()
{
    bWalk = !bWalk;
    if (bWalk)
    {
        bSprint = false; // entering walk cancels sprint
        if (GEngine) GEngine->AddOnScreenDebugMessage(9911, 1.5f, FColor::Yellow, TEXT("Walk: ON"));
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(9911, 1.5f, FColor::Yellow, TEXT("Walk: OFF (Run)"));
    }
}

// Optional click-to-move (works fine with coupled camera)
void AFlyffCharacter::LMB_P()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, Hit.ImpactPoint);
            bAutorun = false;
        }
    }
}

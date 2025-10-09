#include "Mover/Characters/FlyffCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"   // SimpleMoveToLocation (optional LMB click-to-move)
#include "NavigationSystem.h"
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
    SpringArm->bInheritYaw   = true;   // IMPORTANT: was false before
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

    // Make sure the boom actually follows control rotation (not disabled)
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

void AFlyffCharacter::ToggleCamDebug()
{
    bCamDebug = !bCamDebug;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            9998, 2.0f,
            bCamDebug ? FColor::Green : FColor::Red,
            bCamDebug ? TEXT("Camera Debug: ON") : TEXT("Camera Debug: OFF"));
    }
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

    // Speed & autorun
    IC->BindKey(EKeys::LeftShift, IE_Pressed,  this, &AFlyffCharacter::Shift_P);
    IC->BindKey(EKeys::LeftShift, IE_Released, this, &AFlyffCharacter::Shift_R);
    IC->BindKey(EKeys::LeftAlt,   IE_Pressed,  this, &AFlyffCharacter::Alt_P);
    IC->BindKey(EKeys::LeftAlt,   IE_Released, this, &AFlyffCharacter::Alt_R);
    IC->BindKey(EKeys::F,         IE_Pressed,  this, &AFlyffCharacter::F_P);

    // Jump
    IC->BindKey(EKeys::SpaceBar, IE_Pressed,  this, &AFlyffCharacter::Space_P);
    IC->BindKey(EKeys::SpaceBar, IE_Released, this, &AFlyffCharacter::Space_R);

    //debug
    IC->BindKey(EKeys::F9, IE_Pressed, this, &AFlyffCharacter::ToggleCamDebug);
    IC->BindKey(EKeys::Q, IE_Pressed, this, &AFlyffCharacter::DebugYawLeft);
    IC->BindKey(EKeys::E, IE_Pressed, this, &AFlyffCharacter::DebugYawRight);
}

// ---------------- Tick ----------------
void AFlyffCharacter::Tick(float dt)
{
    Super::Tick(dt);
    TickMove(dt);
    TickCamera(dt);
if (bCamDebug && GEngine && SpringArm)
{
    const FRotator BoomRot = SpringArm->GetComponentRotation();
    const float    BoomLen = SpringArm->TargetArmLength;

    const float ActorYaw = GetActorRotation().Yaw;
    float CtrlYaw = 0.f, CtrlPitch = 0.f;
    if (AController* C = GetController())
    {
        const FRotator Ctrl = C->GetControlRotation();
        CtrlYaw = Ctrl.Yaw;
        CtrlPitch = Ctrl.Pitch;
    }

    const float DeltaYaw = FMath::FindDeltaAngleDegrees(CtrlYaw, ActorYaw);

    // On-screen summary (uses a fixed key so it updates in place)
    GEngine->AddOnScreenDebugMessage(9992, 0.f, FColor::Cyan,
        FString::Printf(TEXT("BoomLen=%.1f  Boom(Yaw=%.1f Pitch=%.1f)  Ctrl(Yaw=%.1f Pitch=%.1f)  ActorYaw=%.1f  ΔYaw=%.1f  UsePCR=%d"),
            BoomLen, BoomRot.Yaw, BoomRot.Pitch, CtrlYaw, CtrlPitch, ActorYaw, DeltaYaw,
            SpringArm->bUsePawnControlRotation ? 1 : 0));

    // Draw a line from capsule origin to camera
    const FVector PawnLoc = GetCapsuleComponent() ? GetCapsuleComponent()->GetComponentLocation() : GetActorLocation();
    const FVector CamLoc  = Camera ? Camera->GetComponentLocation() : (PawnLoc + SpringArm->GetForwardVector() * BoomLen);
    DrawDebugLine(GetWorld(), PawnLoc, CamLoc, FColor::Cyan, /*bPersistent*/false, /*LifeTime*/0.f, /*Depth*/0, /*Thickness*/2.f);

    // Draw a little axis at the camera
    DrawDebugDirectionalArrow(GetWorld(), CamLoc, CamLoc + SpringArm->GetForwardVector()*75.f,
                              20.f, FColor::Green, false, 0.f, 0, 2.f);
}
    // quick overlay
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(7, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Speed=%.0f  Fwd=%.1f  Turn=%.1f  Auto=%d"),
            GetCharacterMovement()->MaxWalkSpeed, ForwardAxis, TurnAxis, bAutorun?1:0));
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
    float Target = MoveTune.RunSpeed;
    if (bWalk)   Target = MoveTune.WalkSpeed;
    if (bSprint) Target = MoveTune.SprintSpeed;
    GetCharacterMovement()->MaxWalkSpeed = Target;
}

// ---------------- Camera ----------------
void AFlyffCharacter::TickCamera(float dt)
{
    if (!SpringArm) return;

    // No manual yaw forcing here. The chain is:
    // A/D -> AddControllerYawInput -> Controller Yaw
    // -> Pawn Yaw (bUseControllerRotationYaw)
    // -> Camera Yaw (SpringArm bUsePawnControlRotation + bInheritYaw)

    // Ensure we’re viewing THIS pawn/camera (useful if you swap pawns)
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (Camera && !Camera->IsActive()) Camera->Activate(true);
        if (PC->GetViewTarget() != this)   PC->SetViewTarget(this);
    }

    // Optional debug (safe to keep)
    if (bCamDebug && GEngine)
    {
        const FRotator BoomRot = SpringArm->GetComponentRotation();
        const float    BoomLen = SpringArm->TargetArmLength;

        float CtrlYaw = 0.f, CtrlPitch = 0.f;
        if (AController* C = GetController())
        {
            const FRotator Ctrl = C->GetControlRotation();
            CtrlYaw = Ctrl.Yaw;
            CtrlPitch = Ctrl.Pitch;
        }

        const float ActorYaw = GetActorRotation().Yaw;
        const float DeltaYaw = FMath::FindDeltaAngleDegrees(CtrlYaw, ActorYaw);

        GEngine->AddOnScreenDebugMessage(9992, 0.f, FColor::Cyan,
            FString::Printf(TEXT("BoomLen=%.1f  Boom(Yaw=%.1f Pitch=%.1f)  Ctrl(Yaw=%.1f Pitch=%.1f)  ActorYaw=%.1f  ΔYaw=%.1f  UsePCR=%d"),
                BoomLen, BoomRot.Yaw, BoomRot.Pitch, CtrlYaw, CtrlPitch, ActorYaw, DeltaYaw,
                SpringArm->bUsePawnControlRotation ? 1 : 0));
    }
}



// ---------------- Input handlers ----------------
// WASD
void AFlyffCharacter::W_P(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }
void AFlyffCharacter::W_R(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); }
void AFlyffCharacter::S_P(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); bAutorun=false; }
void AFlyffCharacter::S_R(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }

void AFlyffCharacter::A_P(){ TurnAxis = ClampAxis(TurnAxis - 1.f);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1111, 1.f, FColor::Yellow, TEXT("A_P")); }
void AFlyffCharacter::A_R(){ TurnAxis = ClampAxis(TurnAxis + 1.f);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1112, 1.f, FColor::Yellow, TEXT("A_R")); }
void AFlyffCharacter::D_P(){ TurnAxis = ClampAxis(TurnAxis + 1.f);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1113, 1.f, FColor::Yellow, TEXT("D_P")); }
void AFlyffCharacter::D_R(){ TurnAxis = ClampAxis(TurnAxis - 1.f);
    if (GEngine) GEngine->AddOnScreenDebugMessage(1114, 1.f, FColor::Yellow, TEXT("D_R")); }
//debug
void AFlyffCharacter::DebugYawLeft()  { AddControllerYawInput(-10.f); }
void AFlyffCharacter::DebugYawRight() { AddControllerYawInput(+10.f); }

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
void AFlyffCharacter::Shift_P(){ bSprint = true; }
void AFlyffCharacter::Shift_R(){ bSprint = false; }
void AFlyffCharacter::Alt_P()  { bWalk   = true;  bSprint=false; }
void AFlyffCharacter::Alt_R()  { bWalk   = false; }
void AFlyffCharacter::F_P()    { bAutorun = !bAutorun; }

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
